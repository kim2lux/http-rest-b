#include "handler.hpp"
#include "session.hpp"

Handler::Handler(Session *const session, const std::string_view& host)
    : mSession(session) {
  req_.version(11);
  req_.method(http::verb::get);
  req_.set(http::field::host, host.data());
  req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
}

void Handler::Read() {
  res_ = http::response<http::string_body>{};
  http::async_read(
      mSession->mStream, buffer_, res_,
      beast::bind_front_handler(&Handler::OnRead, shared_from_this()));
}

void Handler::OnRead(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    helper::Fail(ec, "read");

  auto request = mSession->mRequest.get();
  if (request) {
      auto [req, handle] = (*request);
      std::string data{res_.body()};
      handle(data);
  }
  Read();
}

void Handler::OnWrite(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);
  if (ec)
     helper::Fail(ec, "Write error: ");
}

void Handler::Write(const std::string target) {
  req_.target(target);
  http::async_write(
      mSession->mStream, req_,beast::bind_front_handler(&Handler::OnWrite, shared_from_this()));
}

