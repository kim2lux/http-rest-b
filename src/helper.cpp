#include "helper.hpp"

namespace helper {

void Fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

} // namespace helper
