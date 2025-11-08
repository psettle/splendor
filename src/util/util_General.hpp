#ifndef UTIL_GENERAL_HPP
#define UTIL_GENERAL_HPP

#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>

#define __stringize(a) #a
#define stringize(a) __stringize(a)

static bool constexpr kEnableAsserts = true;

#define ASSERT(cond)                                                        \
  do {                                                                      \
    if constexpr (kEnableAsserts) {                                         \
      if (!(cond)) {                                                        \
        throw std::logic_error(__FILE__ ":" stringize(__LINE__) " " #cond); \
      }                                                                     \
    }                                                                       \
  } while (false)

#define ASSERT_ALWAYS() ASSERT(false)

typedef std::uint32_t uint32;
typedef std::uint16_t uint16;
typedef std::uint8_t uint8;
typedef std::uint64_t uint64;

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

template <class Storage, class Callable>
inline void IterateBitfield(Storage bitfield, Callable&& callable) {
  while (bitfield != 0) {
    int offset = __builtin_ctzll(bitfield);
    callable(static_cast<std::size_t>(offset));
    bitfield &= ~(1ull << offset);
  }
}

using Generator = std::mt19937;

static Generator MakeGenerator() {
  std::random_device randomDevice{};
  Generator generator{randomDevice()};
  return generator;
}

template <class T, class GetWeight>
T WeightedSample(std::vector<T> const& aData, GetWeight&& aGetWeight,
                 Generator& aGenerator) {
  ASSERT(aData.size() > 0);

  std::vector<std::size_t> weights{};
  weights.resize(aData.size());

  std::size_t allWeight{0u};
  for (std::size_t i = 0u; i < weights.size(); ++i) {
    weights[i] = aGetWeight(i);
    allWeight += weights[i];
  }

  ASSERT(allWeight > 0u);

  std::size_t sample = aGenerator() % allWeight;
  for (std::size_t i = 0u; i < weights.size(); ++i) {
    if (sample < weights[i]) {
      return aData[i];
    }

    sample -= weights[i];
  }

  ASSERT_ALWAYS();
  return aData.back();
}

}  // namespace util

#endif  // UTIL_GENERAL_HPP