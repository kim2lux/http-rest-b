#pragma once

#include <array>
#include <atomic>
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <utility>

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
  using RequestType = std::pair<std::string, Callback>;

  void add(RequestType request) {
    if (mWritePos - mReadPos >= mSize) {
      // could only return an error
      std::stringstream ss;
      ss << "Write buffer full: " << mWritePos << " " << mReadPos << std::endl;
      throw ss.str();
    }
    mRequests[mWritePos & mRingMask] = request;
    mWritePos++;
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
