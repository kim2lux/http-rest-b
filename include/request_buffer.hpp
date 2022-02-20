#pragma once

#include <array>
#include <atomic>
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <utility>
#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

namespace restclient {
template <typename Callback, size_t SIZE = 4096> class RequestBuffer {
private:
  static constexpr uint64_t LOG2(uint64_t size, int p = 0) {
    return size <= 1 ? p : LOG2(size / 2, p + 1);
  }
  static constexpr uint64_t GetClosestExp(uint64_t size) {
    return 1UL << (LOG2(size - 1) + 1);
  }
  static constexpr uint64_t mSize = GetClosestExp(SIZE);
  static constexpr uint64_t mRingMask = GetClosestExp(SIZE) - 1;

public:
  using RequestType = std::pair<http::request<http::empty_body>, Callback>;

  RequestType &add(http::request<http::empty_body> &httpRequest,
                   std::function<void(std::string &)> hdl) {
    uint64_t currentWritePos = mWritePos.load();
    if (currentWritePos - mReadPos >= mSize) {
      // could only return an error
      std::stringstream ss;
      ss << "Write buffer full: " << currentWritePos << " " << mReadPos
         << std::endl;
      throw ss.str();
    }
    mRequests[currentWritePos & mRingMask] = std::make_pair(httpRequest, hdl);
    mWritePos++;
    return mRequests[currentWritePos & mRingMask];
  }

  std::optional<RequestType> get() {
    uint64_t localReadPos = mReadPos.load(std::memory_order_relaxed);
    do {
      localReadPos = mReadPos.load(std::memory_order_relaxed);
      if (localReadPos >= mWritePos) {
        std::cout << "Read buffer empty: " << (localReadPos & mRingMask)
                  << std::endl;
        return std::nullopt;
      }
    } while (!mReadPos.compare_exchange_strong(localReadPos, localReadPos + 1));
    return mRequests[localReadPos & mRingMask];
  }

  std::array<RequestType, mSize> mRequests;
  std::atomic<uint64_t> mWritePos{0};
  std::atomic<uint64_t> mReadPos{0};
};
} // namespace restclient