#pragma once

#include "helper.hpp"

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class session;

class Handler : public std::enable_shared_from_this<Handler> {
public:
  explicit Handler(session* session);

  ~Handler();
  void start();

  void on_write(beast::error_code ec, std::size_t bytes_transferred);
  void on_read(beast::error_code ec, std::size_t bytes_transferred);
  void do_write(const std::string target);

private:
  beast::flat_buffer buffer_; // (Must persist between reads)
  http::request<http::empty_body> req_;
  http::response<http::string_body> res_;
  std::string mTarget;
  session* mSession;
};