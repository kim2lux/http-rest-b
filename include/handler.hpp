#pragma once

#include "helper.hpp"

#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <string_view>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace restclient {
class Session;

class Handler : public std::enable_shared_from_this<Handler> {
public:
  explicit Handler(Session *const session);

  void Read();
  void OnRead(beast::error_code ec, std::size_t bytes_transferred);

  void OnWrite(beast::error_code ec, std::size_t bytes_transferred);
  void Write(const http::request<http::empty_body> &httpRequest);

  ~Handler() { std::cout << "destroy Handler..." << std::endl; }

private:
  beast::flat_buffer mBuffer;
  http::response<http::string_body> mHttpResult;
  std::string mTarget;
  Session *const mSession;
  bool mHandlerRequestStop{false};
};
} // namespace restclient