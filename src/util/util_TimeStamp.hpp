#ifndef UTIL_TIMESTAMP_HPP
#define UTIL_TIMESTAMP_HPP

#include <chrono>

namespace util {
class TimeStamp {
 public:
  using Time = std::chrono::high_resolution_clock;

  TimeStamp() : t_(Time::now()) {}

  double operator-(TimeStamp const& aOther) const {
    auto diff = t_ - aOther.t_;
    return diff.count() * 0.000000001;
  }

  double Since() const {
    TimeStamp now;
    return now - *this;
  }

 private:
  Time::time_point t_;
};
}  // namespace util

#endif  // UTIL_TIMESTAMP_HPP
