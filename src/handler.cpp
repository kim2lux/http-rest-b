#include "handler.hpp"
#include "session.hpp"

Handler::Handler(std::shared_ptr<session> session, const std::string target)
    : mTarget(target), mSession(session) {
  // Set up an HTTP GET request message
  req_.version(11);
  req_.method(http::verb::get);
  req_.target(target);
  req_.set(http::field::host, mSession->mHost);
  req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
}

Handler::~Handler() {
  std::cout << "handler deleted: " << mTarget << std::endl;
}

void Handler::start() {
  http::async_write(
      mSession->stream_, req_,
      beast::bind_front_handler(&Handler::on_write, shared_from_this()));
}

void Handler::on_write(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    return fail(ec, "write");

  // Receive the HTTP response
  std::cout << "reading response request: " << bytes_transferred << std::endl;
  http::async_read(
      mSession->stream_, buffer_, res_,
      beast::bind_front_handler(&Handler::on_read, shared_from_this()));
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
  res_.clear();

  boost::asio::deadline_timer t(mSession->mIoc, boost::posix_time::seconds(2));
  t.wait();
  std::cout << "sending new request" << std::endl;
  http::async_write(
      mSession->stream_, req_,
      beast::bind_front_handler(&Handler::on_write, shared_from_this()));
}
