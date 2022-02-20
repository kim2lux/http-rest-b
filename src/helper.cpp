#include "helper.hpp"

namespace restclient {

namespace helper {

void Fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << " error value: " << ec.value()
            << "\n";
}

} // namespace helper
} // namespace restclient
