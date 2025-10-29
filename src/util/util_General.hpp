#ifndef UTIL_GENERAL_HPP
#define UTIL_GENERAL_HPP

#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>

#define __stringize(a) #a
#define stringize(a) __stringize(a)

static bool constexpr kEnableAsserts = true;

#define ASSERT(cond)                                                     \
  do {                                                                   \
    if constexpr (kEnableAsserts) {                                      \
      if (!(cond)) {                                                     \
        throw std::logic_error(__FILE__ stringize(__LINE__) ": " #cond); \
      }                                                                  \
    }                                                                    \
  } while (false)

#define ASSERT_ALWAYS() ASSERT(false)

typedef std::uint32_t uint32;
typedef std::uint16_t uint16;
typedef std::uint8_t uint8;

namespace util {

template <class Iterator, class Evaluator>
static Iterator MaxElement(Iterator aBegin, Iterator aEnd,
                           Evaluator&& aEvaluator) {
  auto best = aEnd;
  float bestScore = std::numeric_limits<float>::lowest();

  for (auto it = aBegin; it != aEnd; ++it) {
    float score = aEvaluator(*it);
    if (score > bestScore) {
      best = it;
      bestScore = score;
    }
  }

  return best;
}

using Generator = std::mt19937;

static Generator MakeGenerator() {
  std::random_device randomDevice{};
  Generator generator{randomDevice()};
  return generator;
}

}  // namespace util

#endif  // UTIL_GENERAL_HPP