#pragma once

#include <iostream>

class APIHandlerInterface {
public:
  virtual void ping(std::string& result) const = 0;
  virtual void info(std::string& result) const = 0;
  virtual ~APIHandlerInterface() {}
};

class Decoder final : public APIHandlerInterface {
public:
  explicit Decoder() noexcept {

  }
  void ping(std::string& data) const override {
    // data should be copied
    std::cout << "receive ping: " << data << std::endl;
  }

  void info(std::string& data) const override {
    // data should be copied
    std::cout << "receive info" << data << std::endl;
  }
};