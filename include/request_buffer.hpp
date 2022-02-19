#pragma once

#include <array>
#include <atomic>
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <utility>

template <typename Callback, size_t SIZE = 1024> class RequestBuffer {
public:
  using RequestType = std::pair<std::string, Callback>;
  static constexpr uint64_t LOG2(uint64_t size, int p = 0) {
    return size <= 1 ? p : LOG2(size / 2, p + 1);
  }
  static constexpr uint64_t GetClosestExp(uint64_t size) {
    return 1UL << (LOG2(size - 1) + 1);
  }
  static constexpr uint64_t mSize = GetClosestExp(SIZE);
  static constexpr uint64_t mRingMask = GetClosestExp(SIZE) - 1;

  void add(RequestType request) {
    if (mWritePos - mReadPos >= mSize) {
      //should just return an error, let's throw for now
      std::stringstream ss;
      ss << "Queue full: " << mWritePos << " " << mReadPos << std::endl;
      throw ss.str();
    }
    std::cout << "write position in: " << (mWritePos & mRingMask) << std::endl;
    mRequests[mWritePos & mRingMask] = request;
    mWritePos++;
  }

  std::optional<RequestType> get() {
    uint64_t localReadPos = mReadPos.load(std::memory_order_relaxed);
    std::cout << "try get: " << (localReadPos & mRingMask) << std::endl;
    if (localReadPos >= mWritePos) {
    std::cout << "get NULL ERROR: " << (localReadPos & mRingMask) << std::endl;
      return std::nullopt;
    }
    do {
      localReadPos = mReadPos.load(std::memory_order_relaxed);
      if (localReadPos >= mWritePos) {
    std::cout << "get NULL ERROR: " << (localReadPos & mRingMask) << std::endl;
        return std::nullopt;
      }
    } while (!mReadPos.compare_exchange_strong(localReadPos, localReadPos + 1));
    std::cout << "get: " << (localReadPos & mRingMask) << std::endl;
    return mRequests[localReadPos & mRingMask];
  }

  std::array<RequestType, mSize> mRequests;
  std::atomic<uint64_t> mWritePos{0};
  std::atomic<uint64_t> mReadPos{0};
};
