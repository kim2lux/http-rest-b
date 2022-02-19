#include "root_certificates.hpp"

#include "session.hpp"
#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <future>
#include <functional>

namespace net = boost::asio; // from <boost/asio.hpp>

class Decoder {
public:
  void ping(std::string result) const {
    std::cout << "receive ping: " << result << std::endl;
  }

  void info(std::string result) {
    std::cout << "receive info" << result << std::endl;
  }
};

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
  Decoder decoder;

  // The io_context is required for all I/O
  net::io_context ioc;

  // The SSL context is required, and holds certificates
  ssl::context ctx{ssl::context::tlsv12_client};

  // This holds the root certificate used for verification
  load_root_certificates(ctx);

  // Verify the remote server's certificate
  ctx.set_verify_mode(ssl::verify_peer);

  std::shared_ptr<session> s = std::make_shared<session>(ioc, ctx);
  s->run(host, port, version);
//   auto fut = std::async(std::launch::async, [&]() {
//       while(1) {
//           s->NewRequest("/api/v3/exchangeInfo?symbol=BTCUSDT", std::bind(&Decoder::info, &decoder, std::placeholders::_1));
//           std::this_thread::sleep_for(std::chrono::seconds(3));
//           std::cout << "running " << std::endl;
//       }
//   });

  auto fut2 = std::async(std::launch::async, [&]() {
      while(1) {
          s->NewRequest("/api/v3/ping", [decoder=std::ref(decoder)](const std::string& data) mutable -> void {
              decoder.get().info(data);
          });
          std::this_thread::sleep_for(std::chrono::seconds(3));
          std::cout << "running " << std::endl;
      }
  });
  // Run the I/O service. The call will return when
  // the get operation is complete.
  ioc.run();
  std::cout << "end run io context" << std::endl;
//   fut.wait();
  fut2.wait();

  return EXIT_SUCCESS;
}