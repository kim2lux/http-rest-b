#pragma once

#include "helper.hpp"
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/io_service_strand.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <queue>
#include <memory>
#include "request_buffer.hpp"
#include <functional>

class Handler;

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class Session : public std::enable_shared_from_this<Session> {
public:
  explicit Session(net::io_context& ex, ssl::context &ctx, const std::string& host, const std::string& port);
  void run();
  void AsyncRequest(std::string request, std::function<void(std::string&)>);
  // SyncRequest

private:
  void ExecRequest(typename RequestBuffer<std::function<void(std::string&)>>::RequestType);
  void OnResolve(beast::error_code ec, tcp::resolver::results_type results);
  void OnConnect(beast::error_code ec,
                  tcp::resolver::results_type::endpoint_type);
  void OnHandshake(beast::error_code ec);
  void OnShutdown(beast::error_code ec);

  tcp::resolver mResolver;
public:
  beast::ssl_stream<beast::tcp_stream> mStream;
  net::io_context& mIoc;
  boost::asio::io_context::strand mStrandRequest;
  std::shared_ptr<Handler> mHandler;
  RequestBuffer<std::function<void(std::string&)>> mRequest;
  bool mSessionReady{false};

private:
  const std::string& mHost;
  const std::string& mPort;
};