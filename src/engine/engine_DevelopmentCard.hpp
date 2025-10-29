#ifndef ENGINE_DEVELOPMENTCARD_HPP
#define ENGINE_DEVELOPMENTCARD_HPP

#include <array>
#include <limits>

#include "engine_Gemset.hpp"
#include "util_General.hpp"

namespace engine {

class DevelopmentCard {
 public:
  DevelopmentCard() = default;
  DevelopmentCard(uint8 aIndex) : mIndex(aIndex) {}

  Gemset const& GetCost() const { return ResolveCard(mIndex)->GetCost(); }
  Color GetColor() const { return ResolveCard(mIndex)->GetColor(); }
  size_t GetPoints() const { return ResolveCard(mIndex)->GetPoints(); }

  operator bool() const { return IsValid(); }
  bool IsValid() const { return mIndex != kInvalidDevelopmentCard; }
  void Reset() { mIndex = kInvalidDevelopmentCard; }

  bool operator==(DevelopmentCard const& aOther) const = default;

 private:
  class Internal {
   public:
    Internal(Color aColor, uint8 aPoints, Gemset const& aCost)
        : mCost{aCost}, mPoints{aPoints}, mColor{aColor} {}

    Gemset const& GetCost() const { return mCost; }
    Color GetColor() const { return mColor; }
    size_t GetPoints() const { return mPoints; }

   private:
    Gemset mCost;
    uint8 mPoints;
    Color mColor;
  };

  static Internal const* ResolveCard(uint8 index) {
    if (index < kLevel0.size()) {
      return &kLevel0[index];
    }
    index -= kLevel0.size();

    if (index < kLevel1.size()) {
      return &kLevel1[index];
    }
    index -= kLevel1.size();

    if (index < kLevel2.size()) {
      return &kLevel2[index];
    }

    return nullptr;
  }

  uint8 mIndex{kInvalidDevelopmentCard};

  static uint8 constexpr kInvalidDevelopmentCard =
      std::numeric_limits<uint8>::max();
  static std::array<Internal, 40> const kLevel0;
  static std::array<Internal, 30> const kLevel1;
  static std::array<Internal, 20> const kLevel2;
};

static std::size_t constexpr kDevelopmentCardLevelCount = 3u;

template <std::size_t aDeckSize>
class Deck {
 public:
  Deck(uint8 aOffset) {
    for (std::size_t i = 0u; i < mTop; ++i) {
      mCards[i] = DevelopmentCard(aOffset + i);
    }
  }

  DevelopmentCard Draw(util::Generator& aGenerator) {
    if (mTop == 0u) {
      return DevelopmentCard{};
    }

    uint8 indexToPop = aGenerator() % mTop;
    auto card = mCards[indexToPop];

    for (std::size_t i = 0u; i < mTop - indexToPop - 1u; ++i) {
      mCards[i] = std::move(mCards[i + 1u]);
    }

    return card;
  }

  bool HasCard() const { return mTop > 0u; }

  bool operator==(Deck const& aOther) const {
    if (mTop != aOther.mTop) {
      return false;
    }

    return std::equal(mCards.begin(), mCards.begin() + mTop,
                      aOther.mCards.begin());
  }

 private:
  std::array<DevelopmentCard, aDeckSize> mCards;
  uint8 mTop{aDeckSize};
};

struct Decks {
 public:
  DevelopmentCard Draw(uint8 aLevel, util::Generator& aGenerator) {
    switch (aLevel) {
      case 0:
        return mLevel0.Draw(aGenerator);
      case 1:
        return mLevel1.Draw(aGenerator);
      case 2:
        return mLevel2.Draw(aGenerator);
      default:
        break;
    }

    ASSERT_ALWAYS();
    return DevelopmentCard{};
  }

  bool HasLevel(uint8 aLevel) const {
    switch (aLevel) {
      case 0:
        return mLevel0.HasCard();
      case 1:
        return mLevel1.HasCard();
      case 2:
        return mLevel2.HasCard();
      default:
        break;
    }

    return false;
  }

  bool operator==(Decks const& aOther) const = default;

 private:
  static std::size_t constexpr kLevel0Count{40u};
  static std::size_t constexpr kLevel1Count{30u};
  static std::size_t constexpr kLevel2Count{20u};

  Deck<kLevel0Count> mLevel0{0u};
  Deck<kLevel1Count> mLevel1{kLevel0Count};
  Deck<kLevel2Count> mLevel2{kLevel0Count + kLevel1Count};
};

}  // namespace engine

#endif  // ENGINE_DEVELOPMENTCARD_HPP