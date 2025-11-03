#ifndef ENGINE_GEMSET_HPP
#define ENGINE_GEMSET_HPP

#include <array>

#include "util_General.hpp"

namespace engine {

static std::size_t constexpr kGemColorCount = 5;

enum class Color : uint8 { kWhite, kBlue, kGreen, kRed, kBlack };

class Gemset {
 public:
  Gemset() {}
  Gemset(std::size_t aFirst) {
    for (std::size_t i = 0u; i < kGemColorCount; ++i) {
      Set(i, aFirst);
    }
  }
  Gemset(std::size_t aWhite, std::size_t aBlue, std::size_t aGreen,
         std::size_t aRed, std::size_t aBlack) {
    Set(Color::kWhite, aWhite);
    Set(Color::kBlue, aBlue);
    Set(Color::kGreen, aGreen);
    Set(Color::kRed, aRed);
    Set(Color::kBlack, aBlack);
  }

  std::size_t Get(std::size_t aIndex) const { return mStorage[aIndex]; }
  std::size_t Get(Color aColor) const {
    return Get(static_cast<std::size_t>(aColor));
  }

  void Set(std::size_t aIndex, std::size_t aValue) {
    mStorage[aIndex] = aValue;
  }
  void Set(Color aColor, std::size_t aValue) {
    Set(static_cast<std::size_t>(aColor), aValue);
  }

  static Gemset Add(Gemset const& aLeft, Gemset const& aRight) {
    Gemset diff{};
    for (std::size_t i = 0u; i < kGemColorCount; ++i) {
      diff.Set(i, aLeft.Get(i) + aRight.Get(i));
    }
    return diff;
  }

  static Gemset Sub(Gemset const& aLeft, Gemset const& aRight) {
    Gemset diff{};
    for (std::size_t i = 0u; i < kGemColorCount; ++i) {
      diff.Set(i, aLeft.Get(i) - aRight.Get(i));
    }
    return diff;
  }

  static Gemset ApplyDiscount(Gemset const& aCost, Gemset const& aDiscount) {
    Gemset cost{};

    for (std::size_t i = 0u; i < kGemColorCount; ++i) {
      if (aDiscount.Get(i) < aCost.Get(i)) {
        cost.Set(i, aCost.Get(i) - aDiscount.Get(i));
      }
    }

    return cost;
  }

  /* Returns the number of gold subs needed to afford for each color. */
  static std::size_t GetGoldDemand(Gemset const& aDiscount, Gemset const& aHeld,
                                   Gemset const& aCost) {
    return ApplyDiscount(aCost, Add(aDiscount, aHeld)).GetCount();
  }

  std::size_t constexpr GetCount() const {
    std::size_t count = 0u;

    for (std::size_t i = 0u; i < kGemColorCount; ++i) {
      count += Get(i);
    }

    return count;
  }

  bool operator==(Gemset const& aOther) const = default;

  bool operator<(Gemset const& aOther) const {
    for (std::size_t i = 0u; i < kGemColorCount; ++i) {
      if (Get(i) < aOther.Get(i)) {
        return true;
      }

      if (Get(i) > aOther.Get(i)) {
        return false;
      }
    }

    return false;
  }

 private:
  std::array<uint8, kGemColorCount> mStorage{};
};

}  // namespace engine

#endif  // ENGINE_GEMSET_HPP