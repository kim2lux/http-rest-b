#pragma once

#include <iostream>
namespace restclient {
class APIHandlerInterface {
public:
  virtual void ping(std::string &result) const = 0;
  virtual void info(std::string &result) const = 0;
  virtual ~APIHandlerInterface() {}
};
} // namespace restclient