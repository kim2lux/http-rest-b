#include "session.hpp"
#include "handler.hpp"

session::session(net::io_context &ex, ssl::context &ctx)
    : resolver_(ex), stream_(ex, ctx), mIoc{ex} {}

session::~session() { std::cout << "destruct session" << std::endl; }
// Start the asynchronous operation
void session::run(char const *host, char const *port, int version) {
  mHost = std::string{host};
  mPort = std::string{port};
  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
    beast::error_code ec{static_cast<int>(::ERR_get_error()),
                         net::error::get_ssl_category()};
    std::cerr << ec.message() << "\n";
    return;
  }

  // Look up the domain name
  resolver_.async_resolve(
      host, port,
      beast::bind_front_handler(&session::on_resolve, shared_from_this()));
}

void session::on_resolve(beast::error_code ec,
                         tcp::resolver::results_type results) {
  if (ec)
    return fail(ec, "resolve");

  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
  beast::get_lowest_layer(stream_).async_connect(
      results,
      beast::bind_front_handler(&session::on_connect, shared_from_this()));
}

void session::on_connect(beast::error_code ec,
                         tcp::resolver::results_type::endpoint_type) {
  if (ec)
    return fail(ec, "connect");

  // Perform the SSL handshake
  stream_.async_handshake(
      ssl::stream_base::client,
      beast::bind_front_handler(&session::on_handshake, shared_from_this()));
}

void session::on_handshake(beast::error_code ec) {
  if (ec)
    return fail(ec, "handshake");

  // Set a timeout on the operation
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
  std::make_shared<Handler>(shared_from_this(),
                            std::string{"/api/v3/exchangeInfo?symbol=BTCUSDT"})
      ->start();
}

void session::on_shutdown(beast::error_code ec) {
  if (ec == net::error::eof) {
    ec = {};
  }
  if (ec)
    return fail(ec, "shutdown");
}
