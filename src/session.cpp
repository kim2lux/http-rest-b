#include "session.hpp"
#include "handler.hpp"

Session::Session(net::io_context &ex, ssl::context &ctx,
                 const std::string &host, const std::string &port)
    : mResolver(ex), mStream(ex, ctx), mIoc{ex}, mStrandRequest(ex),
      mHost(host), mPort(port) {
  mHandler = std::make_shared<Handler>(this, mHost.c_str());
}

void Session::run() {
  if (!SSL_set_tlsext_host_name(mStream.native_handle(), mHost.c_str())) {
    beast::error_code ec{static_cast<int>(::ERR_get_error()),
                         net::error::get_ssl_category()};
    std::cerr << ec.message() << "\n";
    return;
  }
  // Look up the domain name
  mResolver.async_resolve(
      mHost, mPort,
      beast::bind_front_handler(&Session::OnResolve, shared_from_this()));
}

void Session::ExecRequest(
    typename RequestBuffer<std::function<void(std::string &)>>::RequestType
        request) {
  mHandler->Write(request.first);
  mRequest.add(request);
}

void Session::AsyncRequest(std::string request,
                           std::function<void(std::string &)> hdl) {
  if (!mSessionReady) {
    std::cout << "Session not ready" << std::endl;
    return;
  }
  boost::asio::post(
      mIoc, mStrandRequest.wrap([me = shared_from_this(), request, hdl]() {
        me->ExecRequest(std::make_pair(request, hdl));
      }));
  return;
}

void Session::OnResolve(beast::error_code ec,
                        tcp::resolver::results_type results) {
  if (ec)
    return helper::Fail(ec, "resolve");
  beast::get_lowest_layer(mStream).expires_after(std::chrono::seconds(30));
  beast::get_lowest_layer(mStream).async_connect(
      results,
      beast::bind_front_handler(&Session::OnConnect, shared_from_this()));
}

void Session::OnConnect(beast::error_code ec,
                        tcp::resolver::results_type::endpoint_type) {
  if (ec)
    return helper::Fail(ec, "connect");
  mStream.async_handshake(
      ssl::stream_base::client,
      beast::bind_front_handler(&Session::OnHandshake, shared_from_this()));
}

void Session::OnHandshake(beast::error_code ec) {
  if (ec)
    return helper::Fail(ec, "handshake");
  beast::get_lowest_layer(mStream).expires_after(std::chrono::seconds(3600));
  std::cout << "Connection is ready for request..." << std::endl;
  mSessionReady = true;
  mHandler->Read();
}

void Session::OnShutdown(beast::error_code ec) {
  if (ec == net::error::eof) {
    ec = {};
  }
  if (ec)
    return helper::Fail(ec, "shutdown");
}
