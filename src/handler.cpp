#include "handler.hpp"
#include "session.hpp"

using namespace restclient;

Handler::Handler(Session *const session) : mSession(session) {
}

void Handler::Read() {

  mHttpResult = http::response<http::string_body>{};
  http::async_read(
      mSession->mStream, mBuffer, mHttpResult,
      beast::bind_front_handler(&Handler::OnRead, shared_from_this()));
}

void Handler::OnRead(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec) {
    return mSession->ErrorHandle(ec, "OnRead");
  }

  //mHttpRequest.method().
  mHttpResult.base();
  auto request = mSession->mRequest.get();
  if (request) {
      auto [req, handle] = (*request);
      handle(mHttpResult.body());
  }
  Read();
}

void Handler::OnWrite(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);
  if (ec) {
    return mSession->ErrorHandle(ec, "OnWrite");
  }
}

void Handler::Write(const http::request<http::empty_body>& httpRequest) {
  http::async_write(
      mSession->mStream, httpRequest, beast::bind_front_handler(&Handler::OnWrite, shared_from_this()));
}
