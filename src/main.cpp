#include "root_certificates.hpp"


#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <future>
#include "session.hpp"

namespace net = boost::asio;      // from <boost/asio.hpp>

int main(int argc, char **argv) {
  // Check command line arguments.
  if (argc != 4 && argc != 5) {
    std::cerr << "Usage: http-client-async-ssl <host> <port> <target> [<HTTP "
                 "version: 1.0 or 1.1(default)>]\n"
              << "Example:\n"
              << "    http-client-async-ssl www.example.com 443 /\n"
              << "    http-client-async-ssl www.example.com 443 / 1.0\n";
    return EXIT_FAILURE;
  }
  auto const host = argv[1];
  auto const port = argv[2];
  auto const target = argv[3];
  int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

  // The io_context is required for all I/O
  net::io_context ioc;

  // The SSL context is required, and holds certificates
  ssl::context ctx{ssl::context::tlsv12_client};

  // This holds the root certificate used for verification
  load_root_certificates(ctx);

  // Verify the remote server's certificate
  ctx.set_verify_mode(ssl::verify_peer);

  // Launch the asynchronous operation
  // The session is constructed with a strand to
  // ensure that handlers do not execute concurrently.
  std::make_shared<session>(ioc, ctx)
      ->run(host, port, version);

  // Run the I/O service. The call will return when
  // the get operation is complete.
  ioc.run();
  std::cout << "end run io context" << std::endl;

  return EXIT_SUCCESS;
}