#ifndef ENGINE_DEVELOPMENTCARD_HPP
#define ENGINE_DEVELOPMENTCARD_HPP

#include <array>
#include <iostream>
#include <limits>
#include <type_traits>

#include "engine_Gemset.hpp"
#include "util_General.hpp"

namespace engine {

class DevelopmentCard {
 public:
  DevelopmentCard() = default;

  Gemset const& GetCost() const { return Resolve()->GetCost(); }
  Color GetColor() const { return Resolve()->GetColor(); }
  size_t GetPoints() const { return Resolve()->GetPoints(); }
  uint8 GetLevel() const {
    if (IsHidden()) {
      switch (GetIndex()) {
        case kHiddenLevel0:
          return 0;
        case kHiddenLevel1:
          return 1;
        case kHiddenLevel2:
          return 2;
        default:
          ASSERT_ALWAYS();
          return 0xFF;
      }
    }

    if (GetIndex() < kLevel0.size()) {
      return 0u;
    }

    if (GetIndex() - kLevel0.size() < kLevel1.size()) {
      return 1u;
    }

    if (GetIndex() - kLevel0.size() - kLevel1.size() < kLevel2.size()) {
      return 2u;
    }

    ASSERT_ALWAYS();
    return 0xFF;
  }

  operator bool() const { return IsValid(); }
  bool IsValid() const { return GetIndex() != kInvalidDevelopmentCard; }
  void Reset() { mIndex = kInvalidDevelopmentCard; }

  bool IsHidden() const {
    return (GetIndex() == kHiddenLevel0 || GetIndex() == kHiddenLevel1 ||
            GetIndex() == kHiddenLevel2);
  }
  DevelopmentCard SetHidden() {
    auto card = *this;
    switch (GetLevel()) {
      case 0:
        SetIndex(kHiddenLevel0);
        break;
      case 1:
        SetIndex(kHiddenLevel1);
        break;
      case 2:
        SetIndex(kHiddenLevel2);
        break;
      default:
        ASSERT_ALWAYS();
        break;
    }
    return card;
  }
  void ClearHidden(DevelopmentCard const& aCard) { SetIndex(aCard.GetIndex()); }

  bool IsRevealed() const { return mIndex & kRevealedBit; }
  void SetRevealed(bool aRevealed) {
    if (aRevealed) {
      mIndex |= kRevealedBit;
    } else {
      mIndex &= ~kRevealedBit;
    }
  }

  bool operator==(DevelopmentCard const& aOther) const {
    return GetIndex() == aOther.GetIndex();
  }

  bool operator<(DevelopmentCard const& aOther) const {
    return GetIndex() < aOther.GetIndex();
  }

  DevelopmentCard(uint8 aIndex) : mIndex(aIndex) {}
  uint8 GetIndex() const { return mIndex & kIndexBits; }

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

  static Internal const* ResolveCard(uint8 aIndex) {
    if (aIndex < kLevel0.size()) {
      return &kLevel0[aIndex];
    }
    aIndex -= kLevel0.size();

    if (aIndex < kLevel1.size()) {
      return &kLevel1[aIndex];
    }
    aIndex -= kLevel1.size();

    if (aIndex < kLevel2.size()) {
      return &kLevel2[aIndex];
    }

    return nullptr;
  }

  Internal const* Resolve() const { return ResolveCard(GetIndex()); }

  void SetIndex(uint8 aIndex) {
    ASSERT(aIndex <= kIndexBits);
    mIndex &= kRevealedBit;
    mIndex |= aIndex;
  }

  uint8 mIndex{kInvalidDevelopmentCard};

  static std::array<Internal, 40> const kLevel0;
  static std::array<Internal, 30> const kLevel1;
  static std::array<Internal, 20> const kLevel2;

  static uint8 constexpr kRevealedBit = 0b10000000;
  static uint8 constexpr kIndexBits = static_cast<uint8>(~kRevealedBit);
  static uint8 constexpr kInvalidDevelopmentCard{kIndexBits};
  static uint8 constexpr kHiddenLevel2 = kInvalidDevelopmentCard - 1u;
  static uint8 constexpr kHiddenLevel1 = kHiddenLevel2 - 1u;
  static uint8 constexpr kHiddenLevel0 = kHiddenLevel1 - 1u;
};

static std::size_t constexpr kDevelopmentCardLevelCount = 3u;

template <std::size_t aDeckSize, std::size_t aOffset>
class Deck {
 public:
  Deck() {
    for (std::size_t i = 0u; i < aDeckSize; ++i) {
      mCards |= (1ul << i);
    }
  }

  DevelopmentCard Draw(util::Generator& aGenerator) {
    if (mCards == 0u) {
      return DevelopmentCard{};
    }

    std::array<uint8, aDeckSize> cards{};
    uint8 top{0u};
    util::IterateBitfield(mCards,
                          [&](std::size_t aIndex) { cards[top++] = aIndex; });

    auto index = cards[aGenerator() % top];

    mCards &= ~(1ul << index);

    return DevelopmentCard{static_cast<uint8>(aOffset + index)};
  }

  void Insert(DevelopmentCard const& aCard) {
    auto index = aCard.GetIndex();
    StorageType bit = (1ul << (index - aOffset));
    ASSERT(!(mCards & bit));
    mCards |= bit;
  }

  bool HasCard() const { return mCards != 0u; }

  bool operator==(Deck const& aOther) const = default;

 private:
  static bool constexpr k64Bit = (aDeckSize > 32);
  using StorageType = std::conditional<k64Bit, uint64, uint32>::type;

  StorageType mCards{};
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

  void Insert(DevelopmentCard const& aCard) {
    switch (aCard.GetLevel()) {
      case 0:
        return mLevel0.Insert(aCard);
      case 1:
        return mLevel1.Insert(aCard);
      case 2:
        return mLevel2.Insert(aCard);
      default:
        break;
    }

    ASSERT_ALWAYS();
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

  Deck<kLevel0Count, 0u> mLevel0{};
  Deck<kLevel1Count, kLevel0Count> mLevel1{};
  Deck<kLevel2Count, kLevel1Count + kLevel0Count> mLevel2{};
};

}  // namespace engine

#endif  // ENGINE_DEVELOPMENTCARD_HPP