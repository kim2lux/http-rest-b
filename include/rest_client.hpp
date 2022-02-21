#pragma once

#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <string>
#include <string_view>
#include <thread>
#include <boost/asio/signal_set.hpp>

#include "decoder.hpp"
#include "helper.hpp"
#include "session.hpp"

namespace net = boost::asio;

namespace restclient {

static constexpr std::string_view cPrefix{"/api/v3/"};

static constexpr uint64_t cMaxAsyncRequests = 20;
static constexpr uint64_t cBinanceLimitRequestsPerMin = 120;
static constexpr uint64_t cTimeoutMs = (1000 / (cBinanceLimitRequestsPerMin / 60));

// assert: is base of decoder interface
template <typename DecoderType> class RestClient {
public:
  static_assert(std::is_base_of_v<APIHandlerInterface<DecoderType>, DecoderType> == true);

  explicit RestClient(const char *host, const char *port, DecoderType &decoder);

  void SubscribeEndpoint(std::string request);
  template <typename F, typename = std::enable_if_t<
                            std::is_invocable_v<F, const std::string &, uint32_t, boost::beast::string_view>>>
  void SubscribeEndpoint(std::string request, F &&fn);
  template <typename... OPTS>
  void SubscribeEndpoint(std::string request, OPTS &&... opts);
  // UnsubscribeEndpoint
  template <typename F, typename = std::enable_if_t<
                            std::is_invocable_v<F, const std::string &, uint32_t, boost::beast::string_view>>>
   RequestStatus AsyncRequest(std::string request, F &&fn);
  void run();

private:
  void LaunchTaskSubscribe() const;
  void SignalHandler(const boost::system::error_code &error, int signal_number);

  net::io_context mIoc;
  ssl::context mCtx{ssl::context::tlsv12_client};
  std::string mHost;
  std::string mPort;
  std::shared_ptr<Session> mSession;
  DecoderType &mDecoder;
  std::vector<std::future<void>> mTasks;
  std::atomic<bool> mStopSubscriber{false};
  boost::asio::signal_set mSignal;
  std::vector<std::pair<std::string, typename Session::HandlerType>> mSubscriber;
  mutable std::future<void> mFutTasks;
};

#include "../src/rest_client.cpp"
} // namespace restclient