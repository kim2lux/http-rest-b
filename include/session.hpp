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

class session : public std::enable_shared_from_this<session> {
public:
  explicit session(net::io_context& ex, ssl::context &ctx);
  ~session();
  void run(char const *host, char const *port, int version);
  void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
  void on_connect(beast::error_code ec,
                  tcp::resolver::results_type::endpoint_type);
  void on_handshake(beast::error_code ec);
  void on_shutdown(beast::error_code ec);

  void ExecRequest(typename RequestBuffer<std::function<void(std::string)>>::RequestType);
  void NewRequest(std::string request, std::function<void(std::string)>);

private:
  tcp::resolver resolver_;

public:
  std::string mHost;
  std::string mPort;
  beast::ssl_stream<beast::tcp_stream> stream_;
  net::io_context& mIoc;
  boost::asio::io_context::strand strand_request;
  std::shared_ptr<Handler> mHandler;
  RequestBuffer<std::function<void(std::string)>> mRequest;
  bool mSessionReady{false};
};