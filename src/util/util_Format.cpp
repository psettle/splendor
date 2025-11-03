
#include "util_Format.hpp"

#include "engine_DevelopmentCard.hpp"
#include "engine_GameState.hpp"
#include "engine_Gemset.hpp"
#include "engine_Move.hpp"
#include "engine_NobleCard.hpp"
#include "engine_Player.hpp"

namespace util {

static void ShowHeader(std::ostream& aOut, std::string const& aText) {
  aOut << "--- " << aText << " ---\n";
}

static char constexpr kPrefix[] = {'W', 'U', 'G', 'R', 'B'};

void ShowGemset(std::ostream& aOut, engine::Gemset const& aSet,
                std::optional<uint8> aGold) {
  for (size_t i = 0u; i < engine::kGemColorCount; ++i) {
    if (i > 0) {
      aOut << " ";
    }
    aOut << kPrefix[i] << ":" << static_cast<uint16>(aSet.Get(i));
  }

  if (aGold.has_value()) {
    aOut << " D:" << static_cast<uint16>(aGold.value());
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
              std::string const& aPrefix, bool aShowHidden) {
  if (aCard.IsHidden()) {
    aOut << "HIDDEN LEVEL " << static_cast<uint8>(aCard.GetLevel() + 1)
         << " CARD";
    return;
  }

  aOut << aPrefix << "points: " << static_cast<uint16>(aCard.GetPoints());
  aOut << ", color: " << kPrefix[static_cast<uint8>(aCard.GetColor())];
  aOut << ", ";
  ShowCost(aOut, aCard.GetCost());
  if (aShowHidden) {
    aOut << ", hidden:" << (aCard.IsRevealed() ? "false" : "true");
  }
}

void ShowPlayer(std::ostream& aOut, engine::Player const& aPlayer) {
  aOut << "points: " << static_cast<uint16>(aPlayer.GetPoints());
  aOut << ", gems: ";
  ShowGemset(aOut, aPlayer.GetHeld(), aPlayer.GetGold());
  aOut << ", purchased: ";
  ShowGemset(aOut, aPlayer.GetDiscount());

  for (auto reserved : aPlayer.GetReservedDevelopmentCards()) {
    if (reserved) {
      bool showHidden = true;
      ShowCard(aOut, reserved, "\n - ", showHidden);
    }
  }
}

void ShowMove(std::ostream& aOut, uint8 aPlayer, engine::Move const& aMove) {
  switch (aMove.mType) {
    case engine::MoveType::kCollect:
      aOut << "collect: ";
      ShowGemset(aOut, aMove.mCollect.mTake);
      break;
    case engine::MoveType::kPurchase:
      aOut << "purchase: ";
      ShowCard(aOut, aMove.mPurchase.mCard);
      break;
    case engine::MoveType::kReserveFaceUp:
      aOut << "reserve: ";
      ShowCard(aOut, aMove.mReserveFaceUp.mCard);
      break;
    case engine::MoveType::kReserveFaceDown:
      aOut << "reserve: RANDOM LEVEL "
           << static_cast<uint16>(aMove.mReserveFaceDown.mLevel + 1) << " CARD";
      break;
    case engine::MoveType::kNoble:
      aOut << "noble: ";
      ShowNoble(aOut, aMove.mNoble.mNoble);
      break;
    case engine::MoveType::kReturn:
      aOut << "return: ";
      ShowGemset(aOut, aMove.mReturn.mGive);
      break;
  }
}

void ShowState(std::ostream& aOut, engine::GameState const& aState) {
  ShowHeader(aOut, "NOBLES");
  for (auto noble : aState.GetNobles()) {
    if (noble) {
      ShowNoble(aOut, noble);
      aOut << "\n";
    }
  }

  auto revealed = aState.GetRevealedDevelopmentCards();
  for (std::size_t l = revealed.size(); l > 0; --l) {
    ShowHeader(aOut, "TIER " + std::to_string(l));
    for (std::size_t i = 0; i < revealed[l - 1].size(); ++i) {
      auto card = revealed[l - 1][i];
      if (card) {
        ShowCard(aOut, card);
        aOut << "\n";
      }
    }
  }

  ShowHeader(aOut, "AVAILABLE");
  util::ShowGemset(aOut, aState.GetAvailable(), aState.GetAvailableGold());
  aOut << "\n";

  std::size_t id = 0u;
  for (auto const& player : aState.GetPlayers()) {
    id++;
    ShowHeader(aOut, "PLAYER " + std::to_string(id));
    ShowPlayer(aOut, player);
    aOut << "\n";
  }

  aOut << '\n';
}

}  // namespace util