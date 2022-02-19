#pragma once

#include "rest_client.hpp"

template <typename DecoderType>
RestClient<DecoderType>::RestClient(const char *host, const char *port, DecoderType &decoder)
    : ctx{ssl::context::tlsv12_client}, mHost{host}, mPort{port},
      mDecoder(decoder) {
  load_root_certificates(ctx);

  // Verify the remote server's certificate
  ctx.set_verify_mode(ssl::verify_peer);
  mSession = std::make_shared<Session>(ioc, ctx, mHost, mPort);
  mSession->run();
}

template <typename DecoderType>
void RestClient<DecoderType>::SubscribeEndpoint(std::string request) {
  std::string newRequest{request.insert(0, cPrefix)};
  mTasks.emplace_back(std::async(std::launch::async, [this, newRequest]() {
    auto callbackFunc = [decoder = std::ref(this->mDecoder)](
                            std::string &data) mutable -> void {
      // dispatch depending of request
      decoder.get().info(data);
    };
    RunTask(newRequest, callbackFunc);
  }));
}

template <typename DecoderType>
template <typename F, typename CallType>
void RestClient<DecoderType>::SubscribeEndpoint(std::string request, F &&fn) {
  std::string newRequest{request.insert(0, cPrefix)};
  mTasks.emplace_back(
      std::async(std::launch::async, [this, newRequest, fn]() -> void {
        RunTask(newRequest, fn);
      }));
}

template <typename DecoderType>
template <typename... OPTS>
void RestClient<DecoderType>::SubscribeEndpoint(std::string request, OPTS &&... opts) {
  std::string newRequest{request.insert(0, cPrefix)};
  newRequest.append(helper::GetTargetOpts(opts...));

  mTasks.emplace_back(std::async(std::launch::async, [this, newRequest]() {
    auto callbackFunc = [decoder = std::ref(this->mDecoder)](
                            std::string &data) mutable -> void {
      // dispatch depending of request
      decoder.get().info(data);
    };
    RunTask(newRequest, callbackFunc);
  }));
}

template <typename DecoderType>
void RestClient<DecoderType>::run() {
  ioc.run();
  for (auto &task : mTasks) {
    task.wait();
  }
}

template <typename DecoderType>
template <typename F>
void RestClient<DecoderType>::RunTask(const std::string &request, F &&fn) const {
  while (true) {
    mSession->AsyncRequest(request, fn);
    std::this_thread::sleep_for(std::chrono::milliseconds(cTimeoutMs));
  }
}
