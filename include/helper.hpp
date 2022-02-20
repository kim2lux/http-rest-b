#pragma once

#include <any>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <iostream>
#include <boost/asio/io_service_strand.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <functional>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace restclient {


enum RequestStatus : uint8_t {
    RequestSent = 1,
    SessionNotReady,
    SessionError,
    EndSubscribeRequest
};

inline std::ostream& operator<<(std::ostream& os, RequestStatus requestStatus) {
    switch (requestStatus) {
        case RequestStatus::SessionNotReady:
          os << "SessionNotReady";
          break;
        case RequestStatus::RequestSent:
          os << "RequestSent";
          break;
        case RequestStatus::SessionError:
          os << "SessionError";
          break;
        case RequestStatus::EndSubscribeRequest:
          os << "SessionError";
          break;
    }
    return os;
}

namespace helper {

// Report a failure
void Fail(beast::error_code ec, char const *what);
template <typename... OPTS> std::string GetTargetOpts(OPTS &&... opts) {
  std::stringstream ss;
  std::vector<std::string> vecOpts = {opts...};

  ss << "?" << vecOpts[0];
  for (unsigned i = 1; i < vecOpts.size(); ++i) {
    ss << "&" << vecOpts[i];
  }
  return ss.str();
}

} // namespace helper
} // namespace restclient