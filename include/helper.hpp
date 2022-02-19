#pragma once

#include <any>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <iostream>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace helper {

// Report a failure
void Fail(beast::error_code ec, char const *what);
template <typename... OPTS> std::string GetTargetOpts(OPTS &&... opts) {
  std::stringstream ss;
  std::vector<std::string> vecOpts = {opts...};

  for (unsigned i = 0; i < vecOpts.size(); ++i) {
    ss << "?" << vecOpts[i];
  }
  return ss.str();
}

} // namespace helper
