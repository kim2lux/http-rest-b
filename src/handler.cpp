#include "handler.hpp"
#include "session.hpp"

Handler::Handler(session *session) : mSession(session) {
  std::cout << "creating new handler" << std::endl;

  // Set up an HTTP GET request message
  req_.version(11);
  req_.method(http::verb::get);
  req_.set(http::field::host, "api.binance.com");
  req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
}

Handler::~Handler() {
  std::cout << "handler deleted: " << mTarget << std::endl;
}

void Handler::start() {
  std::cout << "start handler read" << std::endl;
  res_ = http::response<http::string_body>{};
  http::async_read(
      mSession->stream_, buffer_, res_,
      beast::bind_front_handler(&Handler::on_read, shared_from_this()));
}

void Handler::on_write(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);
  if (ec)
     fail(ec, "write");
}

void Handler::do_write(const std::string target) {
  req_.target(target);
  std::cout << "writing request: " <<  req_ << std::endl;
  http::async_write(
      mSession->stream_, req_,beast::bind_front_handler(&Handler::on_write, shared_from_this()));
}

void Handler::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    fail(ec, "read");

  std::cout << "start msg: " << bytes_transferred << std::endl;
  // Write the message to standard out
  std::cout << boost::beast::make_printable(buffer_.data()) << std::endl;
  std::cout << "body" << std::endl;
  std::cout << res_.body() << std::endl;
  std::cout << "end msg" << std::endl;
  buffer_.consume(buffer_.size());

  boost::asio::deadline_timer t(mSession->mIoc, boost::posix_time::seconds(2));
  auto request = mSession->mRequest.get();
  if (request) {
      auto [req, handle] = (*request);
      std::cout << "calling handler for request: " << req << std::endl;
      std::string data{res_.body()};
      handle(data);
  }
  //   t.wait();
  //   std::cout << "sending new request" << std::endl;
  //   http::async_write(
  //       mSession->stream_, req_,
  //       beast::bind_front_handler(&Handler::on_write, shared_from_this()));
  start();
}
