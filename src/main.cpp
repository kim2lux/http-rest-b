
#include "decoder.hpp"
#include "rest_client.hpp"
#include "session.hpp"

namespace net = boost::asio;
using namespace restclient;

class ClientDecoder final : public APIHandlerInterface {
public:
  explicit ClientDecoder() noexcept {}
  void ping(std::string &data) const override {
    // data should be copied
    std::cout << "receive ping: " << data << std::endl;
  }

  void info(std::string &data) const override {
    // data should be copied
    std::cout << "receive info: " << data << std::endl;
  }
};

void ExamplePingLambda(const std::string &data) {
  // data should be copied
  // callback are holding read async, no extra processing in client handler should be done (same thread)
  std::cout << "receive a ping: " << data << std::endl;
}

void ClientImpl(RestClient<ClientDecoder> &client) {
  auto myAsyncLambda = [](const std::string &data) { ExamplePingLambda(data); };

  while (true) {
    std::cout << "Client Code Implementation sending async request" << std::endl;
    auto status = client.AsyncRequest("ping", myAsyncLambda);
    std::cout << "request status: " << status << std::endl;
    if (status == RequestStatus::SessionError ||
        status == RequestStatus::EndSubscribeRequest) {
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
  }
}

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
    // restClient.SubscribeEndpoint("time");
    restClient.SubscribeEndpoint("trades", "symbol=BTCUSDT", "limit=10");
    // restClient.SubscribeEndpoint("avgPrice","symbol=BTCUSDT");
    // restClient.SubscribeEndpoint("ticker/bookTicker", "symbol=BTCUSDT");
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