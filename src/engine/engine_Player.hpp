#ifndef ENGINE_PLAYER_HPP
#define ENGINE_PLAYER_HPP

#include <array>

#include "engine_DevelopmentCard.hpp"
#include "engine_Gemset.hpp"
#include "util_General.hpp"

namespace engine {

class Player {
 public:
  std::size_t constexpr GetGemCount() const { return mHeld.GetCount() + mGold; }

  std::size_t GetPoints() const { return mPoints; }

  void AddTurn() { mTurnCount++; }
  std::size_t GetTurnCount() const { return mTurnCount; }

  uint8 GetDevelopmentCardCount() const { return mDiscount.GetCount(); }

  void AddGems(Gemset const& aTake);
  void RemoveGems(Gemset const& aRemove);

  void AddDevelopmentCard(DevelopmentCard aCard, bool aRevealed);
  DevelopmentCard RemoveDevelopmentCard(uint8 aIndex);

  auto const& GetReservedDevelopmentCards() const { return mReserved; }
  auto& GetReservedDevelopmentCards() { return mReserved; }

  uint8 GetGold() const { return mGold; }
  void AddGold(uint8 aCount) { mGold += aCount; }
  void RemoveGold(uint8 aCount) {
    ASSERT(mGold >= aCount);
    mGold -= aCount;
  }
  Gemset const& GetHeld() const { return mHeld; }
  Gemset const& GetDiscount() const { return mDiscount; }

  void AddPoints(std::size_t aPoints) { mPoints += aPoints; }
  void AddDiscount(Color aColor) {
    mDiscount.Set(aColor, mDiscount.Get(aColor) + 1u);
  }

  enum class TurnPhase : uint8 { kAction, kReturn, kNoble };
  TurnPhase GetPhase() const { return mPhase; }
  void SetPhase(TurnPhase aPhase) { mPhase = aPhase; }

  bool operator==(Player const& aOther) const = default;

 private:
  static std::size_t constexpr kReservedCardMaxCount{3u};

  uint8 mTurnCount{0u};
  Gemset mHeld{};
  Gemset mDiscount{};
  std::array<DevelopmentCard, kReservedCardMaxCount> mReserved{};
  uint8 mGold{0u};
  uint8 mPoints{0u};
  TurnPhase mPhase{TurnPhase::kAction};
};

}  // namespace engine

#endif  // ENGINE_PLAYER_HPP