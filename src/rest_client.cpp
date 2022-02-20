#pragma once

#include "rest_client.hpp"
#include <future>
using namespace restclient;

template <typename DecoderType>
RestClient<DecoderType>::RestClient(const char *host, const char *port,
                                    DecoderType &decoder)
    : mCtx{ssl::context::tlsv12_client}, mHost{host}, mPort{port},
      mDecoder(decoder), mSignal{mIoc, SIGINT} {

  // Start an asynchronous wait for one of the signals to occur.
  mSignal.async_wait(std::bind(&RestClient<DecoderType>::SignalHandler, this,
                               std::placeholders::_1, std::placeholders::_2));
  load_root_certificates(mCtx);

  // Verify the remote server's certificate
  mCtx.set_verify_mode(ssl::verify_peer);
  mSession = std::make_shared<Session>(mIoc, mCtx, mHost, mPort);
  std::cout << "timeout per message: " << cTimeoutMs << std::endl;
  mSession->run();
}

template <typename DecoderType>
void RestClient<DecoderType>::SignalHandler(
    [[maybe_unused]] const boost::system::error_code &error, [[maybe_unused]] int signal_number) {
  mStopSubscriber.store(true);
  std::cout << "Stopping session" << std::endl;
  mSession->Stop();
}

template <typename DecoderType>
void RestClient<DecoderType>::SubscribeEndpoint(std::string request) {
  std::string newRequest{request.insert(0, cPrefix)};
    auto callbackFunc = [decoder = std::ref(this->mDecoder)](
                            std::string &data) mutable -> void {
      // dispatch depending of request
      decoder.get().info(data);
    };
  mSubscriber.emplace_back(std::make_pair(newRequest,callbackFunc));
}

template <typename DecoderType>
template <typename F, typename CallType>
void RestClient<DecoderType>::SubscribeEndpoint(std::string request, F &&fn) {
  std::string newRequest{request.insert(0, cPrefix)};
  mSubscriber.emplace_back(std::make_pair(newRequest,fn));
}

template <typename DecoderType>
template <typename... OPTS>
void RestClient<DecoderType>::SubscribeEndpoint(std::string request,
                                                OPTS &&... opts) {
  std::string newRequest{request.insert(0, cPrefix)};
  newRequest.append(helper::GetTargetOpts(opts...));
    auto callbackFunc = [decoder = std::ref(this->mDecoder)](
                            std::string &data) mutable -> void {
      // dispatch depending of request
      decoder.get().info(data);
    };
  mSubscriber.emplace_back(std::make_pair(newRequest,callbackFunc));
}

template <typename DecoderType> void RestClient<DecoderType>::run() {
  LaunchTaskSubscribe();
  mIoc.run();
  mFutTasks.wait_for(std::chrono::milliseconds{1000});
}

template <typename DecoderType>
void RestClient<DecoderType>::LaunchTaskSubscribe() const {
  mFutTasks = std::async(std::launch::async, [this]() -> void {
    while (!this->mStopSubscriber && !this->mSession->mError) {
      for (const auto &task : this->mSubscriber) {
        const auto &[request, fn] = task;
        this->mSession->AsyncRequest(request, fn);
        // sleep per message, should use automatic throttle in async request
        std::this_thread::sleep_for(std::chrono::milliseconds(cTimeoutMs));
      }
    }
  });
}
