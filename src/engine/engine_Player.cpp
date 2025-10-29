
#include "engine_Player.hpp"

namespace engine {

void Player::AddGems(Gemset const& aTake) { mHeld = Gemset::Add(mHeld, aTake); }

void Player::RemoveGems(Gemset const& aRemove) {
  for (std::size_t i = 0u; i < kGemColorCount; ++i) {
    ASSERT(mHeld.Get(i) >= aRemove.Get(i));
  }

  mHeld = Gemset::Sub(mHeld, aRemove);
}

void Player::AddDevelopmentCard(DevelopmentCard aCard) {
  for (auto& slot : mReserved) {
    if (!slot) {
      slot = aCard;
      return;
    }
  }

  ASSERT_ALWAYS();
}

DevelopmentCard Player::RemoveDevelopmentCard(uint8 aIndex) {
  auto card = mReserved[aIndex];
  mReserved[aIndex].Reset();
  return card;
}

}  // namespace engine