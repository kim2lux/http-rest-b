

#include "rest_client.hpp"
#include "client_decoder_example.hpp"

namespace net = boost::asio;
using namespace restclient;

int main(int argc, char **argv) {
  // Check command line arguments.
  try {
    if (argc != 3) {
      std::cerr << "Usage: ./rest-client <host> <port>"
                << "    ./rest-client api.binance.com 443 /\n";
      return EXIT_FAILURE;
    }

    // simple decoder example
    ClientDecoder decoder;

    RestClient<ClientDecoder> restClient(argv[1], argv[2], decoder);
    restClient.SubscribeEndpoint("exchangeInfo", "symbol=BTCUSDT");
    restClient.SubscribeEndpoint("time");
    restClient.SubscribeEndpoint("trades", "symbol=BTCUSDT", "limit=10");
    restClient.SubscribeEndpoint("avgPrice","symbol=BTCUSDT");
    restClient.SubscribeEndpoint("ticker/bookTicker", "symbol=BTCUSDT");
    std::future<void> clientFuture = std::async(std::launch::async, [&restClient]() {
        ClientImpl(restClient);
    });
    restClient.run();
    clientFuture.wait_for(std::chrono::milliseconds{1000});
  }
  catch (boost::beast::system_error& e) {
    std::cout << "boost::beast error: " << e.what() << std::endl;
  }
  catch (std::runtime_error &e) {
    std::cout << "runtime_error: " << e.what() << std::endl;
  }
  catch (std::exception &e) {
    std::cout << "exception: " << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}