
#include "decoder.hpp"
#include "rest_client.hpp"
#include "session.hpp"

namespace net = boost::asio; // from <boost/asio.hpp>

void ExamplePingLambda(const std::string &data) {
  // data should be copied
  // callback are holding read async, no extra processing in client handler should be done (same thread)
  std::cout << "receive a ping: " << data << std::endl;
}

int main(int argc, char **argv) {
  // Check command line arguments.
  try {
    if (argc != 4) {
      std::cerr << "Usage: http-client-async-ssl <host> <port> <target> [<HTTP "
                   "version: 1.0 or 1.1(default)>]\n"
                << "Example:\n"
                << "    http-client-async-ssl www.example.com 443 /\n"
                << "    http-client-async-ssl www.example.com 443 / 1.0\n";
      return EXIT_FAILURE;
    }

    // decoder example
    Decoder decoder;
    // local lambda example
    auto myAsyncLambda = [](const std::string& data){
        ExamplePingLambda(data);
    };

    RestClient<Decoder> client(argv[1], argv[2], decoder);

    client.SubscribeEndpoint("ping", myAsyncLambda);
    client.SubscribeEndpoint("exchangeInfo", "symbol=BTCUSDT");
    client.run();
  } catch (std::exception &e) {
    std::cout << "error: " << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}