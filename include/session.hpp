#pragma once

#include <memory>
#include "helper.hpp"
#include "request_buffer.hpp"
#include "handler.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace restclient {
class Handler;
class Session : public std::enable_shared_from_this<Session> {
public:
  explicit Session(net::io_context &ex, ssl::context &ctx,
                   const std::string &host, const std::string &port);
  void run();
  RequestStatus AsyncRequest(std::string request, std::function<void(std::string &)>);

  void Stop() {
    mIoc.stop();
  }

  ~Session() { std::cout << "Destroy Session..." << std::endl; }

  void ErrorHandle(beast::error_code &error, const char *ctx) {
    std::cout << "Error occured in context:" << ctx << ", "
              << error.category().name() << ": " << error.message()
              << std::endl;
    mError = error;
    mIoc.stop();
  }

private:
  void ExecRequest(std::string request, std::function<void(std::string &)>);
  void OnResolve(beast::error_code ec, tcp::resolver::results_type results);
  void OnConnect(beast::error_code ec,
                 tcp::resolver::results_type::endpoint_type);
  void OnHandshake(beast::error_code ec);
  void OnShutdown(beast::error_code ec);
  tcp::resolver mResolver;

public:
  beast::ssl_stream<beast::tcp_stream> mStream;
  net::io_context &mIoc;
  boost::asio::io_context::strand mStrandRequest;
  std::shared_ptr<Handler> mHandler;
  RequestBuffer<std::function<void(std::string &)>> mRequest;
  bool mSessionReady{false};
  beast::error_code mError{};

private:
  const std::string &mHost;
  const std::string &mPort;
  http::request<http::empty_body> mHttpRequest;
};
} // namespace restclient