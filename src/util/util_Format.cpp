
#include "util_Format.hpp"

#include "engine_DevelopmentCard.hpp"
#include "engine_GameState.hpp"
#include "engine_Gemset.hpp"
#include "engine_Move.hpp"
#include "engine_NobleCard.hpp"
#include "engine_Player.hpp"

namespace util {

static char constexpr kPrefix[] = {'W', 'U', 'G', 'R', 'B'};

void ShowGemset(std::ostream& aOut, engine::Gemset const& aSet,
                std::optional<uint8> aGold) {
  for (size_t i = 0u; i < engine::kGemColorCount; ++i) {
    aOut << kPrefix[i] << ":" << static_cast<uint16>(aSet.Get(i)) << " ";
  }

  if (aGold.has_value()) {
    aOut << "D:" << static_cast<uint16>(aGold.value());
  }
}

void ShowCost(std::ostream& aOut, engine::Gemset const& aCost,
              std::optional<uint8> aGold) {
  aOut << "cost: ";
  ShowGemset(aOut, aCost, aGold);
}

void ShowNoble(std::ostream& aOut, engine::NobleCard const& aNoble) {
  aOut << "points: " << static_cast<uint16>(aNoble.GetPoints()) << ", ";
  ShowCost(aOut, aNoble.GetCost());
}

void ShowCard(std::ostream& aOut, engine::DevelopmentCard const& aCard,
              std::string const& aPrefix) {
  aOut << aPrefix << "points: " << static_cast<uint16>(aCard.GetPoints());
  aOut << ", color: " << kPrefix[static_cast<uint8>(aCard.GetColor())];
  aOut << ", ";
  ShowCost(aOut, aCard.GetCost());
}

void ShowPlayer(std::ostream& aOut, engine::Player const& aPlayer) {
  aOut << "points: " << static_cast<uint16>(aPlayer.GetPoints());
  aOut << ", gems: ";
  ShowGemset(aOut, aPlayer.GetHeld(), aPlayer.GetGold());
  aOut << ", purchased: ";
  ShowGemset(aOut, aPlayer.GetDiscount());

  for (auto reserved : aPlayer.GetReservedDevelopmentCards()) {
    if (reserved) {
      ShowCard(aOut, reserved, "\n - ");
    }
  }
}

void ShowMove(std::ostream& aOut, engine::GameState const& aState,
              uint8 aPlayer, engine::Move const& aMove) {
  switch (aMove.mType) {
    case engine::MoveType::kCollect:
      aOut << "collect: ";
      ShowGemset(aOut, aMove.mCollect.mTake);
      break;
    case engine::MoveType::kPurchase:
      aOut << "purchase: ";
      if (aMove.mPurchase.mLevel < engine::kDevelopmentCardLevelCount) {
        ShowCard(aOut,
                 aState.GetRevealedDevelopmentCards()[aMove.mPurchase.mLevel]
                                                     [aMove.mPurchase.mIndex]);
      } else {
        ShowCard(aOut,
                 aState.GetPlayers()[aPlayer]
                     .GetReservedDevelopmentCards()[aMove.mPurchase.mIndex]);
      }
      break;
    case engine::MoveType::kReserveFaceUp:
      aOut << "reserve: ";
      ShowCard(
          aOut,
          aState.GetRevealedDevelopmentCards()[aMove.mReserveFaceUp.mLevel]
                                              [aMove.mReserveFaceUp.mIndex]);
      break;
    case engine::MoveType::kReserveFaceDown:
      aOut << "reserve: RANDOM LEVEL "
           << static_cast<uint16>(aMove.mReserveFaceDown.mLevel) << " CARD";
      break;
    case engine::MoveType::kNoble:
      aOut << "noble: ";
      ShowNoble(aOut, aState.GetNobles()[aMove.mNoble.mIndex]);
      break;
    case engine::MoveType::kReturn:
      aOut << "return: ";
      ShowGemset(aOut, aMove.mReturn.mGive);
      break;
  }
}

}  // namespace util