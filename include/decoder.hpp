#pragma once

#include "boost/beast/core.hpp"
#include <iostream>
#include <string_view>

static constexpr boost::beast::string_view ApiPingEvent = "/api/v3/ping";
static constexpr boost::beast::string_view ApiTimeEvent = "/api/v3/time";
static constexpr boost::beast::string_view ApiInfoEvent = "/api/v3/exchangeInfo";
static constexpr boost::beast::string_view ApiTradeEvent = "/api/v3/trades";

namespace restclient {
template <class T> class APIHandlerInterface {
public:
  void dispatch(const std::string &data, uint32_t result,
                boost::beast::string_view target) {
    // Todo: Could convert to json before passing to client handler
    // Define and Switch on Enum message to avoid if msgType, else if msgType
    // ... handle all message type, use DecodeOther for now
    std::cout << "***********" << std::endl;
    std::cout << "target: " << target << std::endl;
    std::cout << "result: " << result << std::endl;
    if (target.find(ApiPingEvent) != boost::string_view::npos) {
      static_cast<T *>(this)->DecodePing(data);
    } else if (target.find(ApiInfoEvent) != boost::string_view::npos) {
      static_cast<T *>(this)->Decodeinfo(data);
    } else if (target.find(ApiTimeEvent) != boost::string_view::npos) {
      static_cast<T *>(this)->DecodeTime(data);
    } else if (target.find(ApiTradeEvent) != boost::string_view::npos) {
      static_cast<T *>(this)->DecodeTrade(data);
    } else {
      static_cast<T *>(this)->DecodeOther(data);
    }
    std::cout << "***********" << std::endl;
  }
  virtual void DecodePing(const std::string &data) const = 0;
  virtual void DecodeTime(const std::string &data) const = 0;
  virtual void DecodeTrade(const std::string &data) const = 0;
  virtual void Decodeinfo(const std::string &data) const = 0;
  virtual void DecodeOther(const std::string &data) const = 0;
  virtual ~APIHandlerInterface() {}
};
} // namespace restclient