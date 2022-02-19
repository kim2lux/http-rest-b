#pragma once

#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <string_view>
#include <stdint.h>

#include "helper.hpp"
#include "root_certificates.hpp"
#include "session.hpp"

namespace net = boost::asio; // from <boost/asio.hpp>
static constexpr std::string_view cPrefix{"/api/v3/"};
static constexpr uint64_t cBinanceLimitRequestPerMin = 60;
static constexpr uint64_t cTimeoutMs = 1000 / (cBinanceLimitRequestPerMin / 60);


// assert: is base of decoder interface
template <typename DecoderType> class RestClient {
public:
  explicit RestClient(const char *host, const char *port, DecoderType &decoder);

  void SubscribeEndpoint(std::string request);

  template <typename F, typename = std::enable_if_t<std::is_invocable_v<F, const std::string&>>>
  void SubscribeEndpoint(std::string request, F &&fn);
  template <typename... OPTS>
  void SubscribeEndpoint(std::string request, OPTS&& ... opts);

  void run();

private:
  template <typename F>
  void RunTask(const std::string &request, F&& fn) const;

  net::io_context ioc;
  ssl::context ctx{ssl::context::tlsv12_client};
  std::string mHost;
  std::string mPort;
  std::shared_ptr<Session> mSession;
  DecoderType &mDecoder;
  std::vector<std::future<void>> mTasks;
};

#include "../src/rest_client.cpp"
