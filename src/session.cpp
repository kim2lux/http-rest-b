#include "session.hpp"
#include "handler.hpp"
#include <thread>
using namespace restclient;

namespace net = boost::asio;
Session::Session(net::io_context &ex, ssl::context &ctx,
                 const std::string &host, const std::string &port)
    : mResolver(net::make_strand(ex)),
      mStream(net::make_strand(ex), ctx), mIoc{ex}, mStrandRequest(ex),
      mHost(host), mPort(port) {
  mHandler = std::make_shared<Handler>(this);

  // prepare request
  mHttpRequest.version(11);
  mHttpRequest.method(http::verb::get);
  mHttpRequest.set(http::field::host, mHost.data());
  mHttpRequest.keep_alive(true);
  mHttpRequest.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
}
void Session::ErrorHandle(beast::error_code &error, const char *ctx) {
  std::cout << "Error occured in context " << ctx << " : "
            << error.category().name() << ": " << error.message() << std::endl;
  mError = error;
  mIoc.stop();
}
void Session::Run() {
  if (!SSL_set_tlsext_host_name(mStream.native_handle(), mHost.c_str())) {
    beast::error_code ec{static_cast<int>(::ERR_get_error()),
                         net::error::get_ssl_category()};
    std::cerr << ec.message() << std::endl;
    return;
  }
  // Look up the domain name
  mResolver.async_resolve(
      mHost, mPort,
      beast::bind_front_handler(&Session::OnResolve, shared_from_this()));
}

void Session::ExecRequest(std::string request,
                          typename Session::HandlerType hdl) {
  mHttpRequest.target(request);
  mHandler->Write(mRequest.add(mHttpRequest, hdl).first);
}

RequestStatus Session::AsyncRequest(std::string request,
                                    typename Session::HandlerType hdl) {
  if (!mSessionReady) {
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    return RequestStatus::SessionNotReady;
  }

  // Use post to synchronize writing thread:
  // Make sure that only one thread can access ExecRequest as only a single
  // Instance is running the context
  // implement throttle to handle api limit; 20 msg/sec
  // if throttle.TryStamp() == Throttle::OK {}
  // -> circular buffer std::array<requestTimeStamp, 20>, check
  // (currenttimestamp - (writer + 1)timestamp) < 1000ms ? yield wait : exec
  // request
  boost::asio::post(
      mIoc, mStrandRequest.wrap([me = shared_from_this(), request, hdl]() {
        me->ExecRequest(request, hdl);
      }));
  return RequestStatus::RequestSent;
}

void Session::OnResolve(beast::error_code ec,
                        tcp::resolver::results_type results) {
  if (ec) {
    return ErrorHandle(ec, "resolve");
  }
  beast::get_lowest_layer(mStream).expires_after(std::chrono::seconds(5));
  beast::get_lowest_layer(mStream).async_connect(
      results,
      beast::bind_front_handler(&Session::OnConnect, shared_from_this()));
}

void Session::OnConnect(beast::error_code ec,
                        tcp::resolver::results_type::endpoint_type) {
  if (ec) {
    return ErrorHandle(ec, "OnConnect");
  }
  std::cout << "Connected to host: " << mHost << std::endl;
  mStream.async_handshake(
      ssl::stream_base::client,
      beast::bind_front_handler(&Session::OnHandshake, shared_from_this()));
}

void Session::OnHandshake(beast::error_code ec) {
  if (ec) {
    return this->ErrorHandle(ec, "OnHandshake");
  }
  beast::get_lowest_layer(mStream).expires_never();
  std::cout << "Ready to start request." << std::endl;
  mSessionReady = true;
  mHandler->Read();
}

void Session::OnShutdown(beast::error_code ec) {
  std::cout << "Stream has been shut down" << std::endl;
  if (ec == net::error::eof) {
    ec = {};
  }
  if (ec) {
    return this->ErrorHandle(ec, "OnShutdown");
  }
}

void Session::Stop() {
    mIoc.stop();
}