#pragma once

#include "decoder.hpp"
using namespace restclient;

class ClientDecoder final : public APIHandlerInterface<ClientDecoder> {
public:
  explicit ClientDecoder() noexcept {}

  // data should be copied
  // callback are holding read async, no extra processing in client handler
  // should be done (same thread)

  void DecodePing(const std::string &data) const override {
    // data should be copied
    std::cout << "DecodePing: " << data << std::endl;
  }

  void Decodeinfo(const std::string &data) const override {
    // data should be copied
    std::cout << "Decodeinfo: " << data << std::endl;
  }

  void DecodeTrade(const std::string &data) const override {
    // data should be copied
    std::cout << "DecodeTrade: " << data << std::endl;
  }

  void DecodeTime(const std::string &data) const override {
    // data should be copied
    std::cout << "DecodeTime: " << data << std::endl;
  }

  void DecodeOther(const std::string &data) const override {
    // data should be copied
    std::cout << "DecodeOther: " << data << std::endl;
  }
};

void ExamplePingLambda(const std::string &data, uint32_t result,
                       boost::beast::string_view target) {
  std::cout << "*** Client Handler ****"
            << "receive ping: " << data << std::endl
            << "result: " << result << std::endl
            << "target: " << target << std::endl;
}

void ClientImpl(RestClient<ClientDecoder> &client) {
  auto myAsyncLambda = [](const std::string &data, uint32_t result,
                          boost::beast::string_view target) {
    ExamplePingLambda(data, result, target);
  };

  std::cout << "Client Code Implementation sending async request" << std::endl;
  while (true) {
    auto status = client.AsyncRequest("ping", myAsyncLambda);
    if (status == RequestStatus::SessionError ||
        status == RequestStatus::EndSubscribeRequest) {
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
  }
}