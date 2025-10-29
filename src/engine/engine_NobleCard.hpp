#ifndef ENGINE_NOBLECARD_HPP
#define ENGINE_NOBLECARD_HPP

#include <array>
#include <limits>

#include "engine_Gemset.hpp"
#include "util_General.hpp"

namespace engine {

class NobleCard {
 public:
  NobleCard() = default;
  NobleCard(uint8 aIndex) : mIndex(aIndex) {}

  uint8 GetPoints() const { return ResolveCard(mIndex)->GetPoints(); }
  Gemset const& GetCost() const { return ResolveCard(mIndex)->GetCost(); }

  operator bool() const { return IsValid(); }
  bool IsValid() const { return mIndex != kInvalidNobleCard; }
  void Reset() { mIndex = kInvalidNobleCard; }

  static std::size_t constexpr kRevealedNobleCount{3u};
  static std::array<NobleCard, kRevealedNobleCount> ShuffleNobles(
      util::Generator& aGenerator);

  bool operator==(NobleCard const& aOther) const = default;

 private:
  class Internal {
   public:
    Internal(Gemset const& aCost) : mCost(aCost) {}

    uint8 GetPoints() const { return 3u; }
    Gemset const& GetCost() const { return mCost; }

   private:
    Gemset mCost{};
  };

  static Internal const* ResolveCard(uint8 aIndex) {
    if (aIndex < kAllNobleCards.size()) {
      return &kAllNobleCards[aIndex];
    }

    return nullptr;
  }

  static uint8 constexpr kInvalidNobleCard = std::numeric_limits<uint8>::max();
  static std::array<Internal, 10u> const kAllNobleCards;

  uint8 mIndex{kInvalidNobleCard};
};

}  // namespace engine

#endif  // ENGINE_NOBLECARD_HPP