#include "engine_GameState.hpp"

#include <algorithm>

#include "engine_Move.hpp"

namespace engine {

GameState::GameState(Generator& aGenerator) {
  for (std::size_t level = 0; level < mRevealed.size(); ++level) {
    for (std::size_t index = 0; index < mRevealed[level].size(); ++index) {
      ReplaceCard(level, index, aGenerator);
    }
  }

  mNobles = NobleCard::ShuffleNobles(aGenerator);
  mNextPlayer = aGenerator() % mPlayers.size();
}

std::vector<Move> GameState::GetMoves() const {
  ASSERT(mDeterminized);

  if (IsTerminal()) {
    return std::vector<Move>();
  }

  std::vector<Move> moves{};
  moves.reserve(45u);

  if (GetPlayer().GetPhase() == Player::TurnPhase::kReturn) {
    GetReturnMoves(moves);
  } else if (GetPlayer().GetPhase() == Player::TurnPhase::kNoble) {
    GetNobleMoves(moves);
  } else {
    GetCollectMoves(moves);
    GetPurchaseMoves(moves);
    GetReserveMoves(moves);
  }

  ASSERT(moves.size() > 0);
  return moves;
}

std::optional<uint8> GameState::GetWinner() const {
  ASSERT(IsTerminal());

  Player const& left = mPlayers.front();
  Player const& right = mPlayers.back();

  if (left.GetPoints() == right.GetPoints()) {
    if (left.GetDevelopmentCardCount() > right.GetDevelopmentCardCount()) {
      return 0;
    } else if (left.GetDevelopmentCardCount() <
               right.GetDevelopmentCardCount()) {
      return 1;
    } else {
      return std::nullopt;
    }
  }

  if (left.GetPoints() > right.GetPoints()) {
    return 0;
  } else if (left.GetPoints() < right.GetPoints()) {
    return 1;
  } else {
    return std::nullopt;
  }
}

bool GameState::IsTerminal() const {
  if (mPlayers.front().GetTurnCount() != mPlayers.back().GetTurnCount()) {
    return false;
  }

  if (mPlayers.front().GetTurnCount() > kMaxTurnCount) {
    return true;
  }

  for (auto const& player : mPlayers) {
    if (player.GetPoints() >= kWinningPointCount) {
      return true;
    }
  }

  return false;
}

void GameState::DoMove(Move const& aMove, Generator& aGenerator) {
  ASSERT(mDeterminized);

  switch (aMove.mType) {
    case MoveType::kCollect:
      DoCollectMove(aMove.mCollect.mTake);
      break;
    case MoveType::kPurchase:
      DoPurchaseMove(aMove.mPurchase.mCard, aGenerator);
      break;
    case MoveType::kReserveFaceUp:
      DoReserveFaceUpMove(aMove.mReserveFaceUp.mCard, aGenerator);
      break;
    case MoveType::kReserveFaceDown:
      DoReserveFaceDownMove(aMove.mReserveFaceDown.mLevel, aGenerator);
      break;
    case MoveType::kNoble:
      DoNobleMove(aMove.mNoble.mNoble);
      break;
    case MoveType::kReturn:
      DoReturnMove(aMove.mReturn.mGive);
      break;
    default:
      ASSERT_ALWAYS();
      break;
  }

  bool endTurn = false;
  auto& player = GetPlayer();

  switch (player.GetPhase()) {
    case Player::TurnPhase::kAction:
      if (player.GetGemCount() > kMaxGemCount) {
        player.SetPhase(Player::TurnPhase::kReturn);
      } else if (HasNobleMoves()) {
        player.SetPhase(Player::TurnPhase::kNoble);
      } else {
        endTurn = true;
      }
      break;
    case Player::TurnPhase::kReturn:
      if (HasNobleMoves()) {
        player.SetPhase(Player::TurnPhase::kNoble);
      } else {
        endTurn = true;
      }
      break;
    case Player::TurnPhase::kNoble:
      endTurn = true;
      break;
    default:
      ASSERT_ALWAYS();
      break;
  }

  if (endTurn) {
    player.SetPhase(Player::TurnPhase::kAction);
    player.AddTurn();
    mNextPlayer = 1 - mNextPlayer;
  }
}

void GameState::DoCollectMove(Gemset const& aTake) {
  ASSERT(aTake.GetCount() <= 3);

  for (std::size_t i = 0; i < kGemColorCount; ++i) {
    ASSERT(aTake.Get(i) <= 2);
    if (aTake.Get(i) == 2) {
      ASSERT(mAvailable.Get(i) == 4u);
    }
    ASSERT(aTake.Get(i) <= mAvailable.Get(i));
  }

  GetPlayer().AddGems(aTake);
  mAvailable = Gemset::Sub(mAvailable, aTake);
}

void GameState::DoPurchaseMove(DevelopmentCard const& aCard,
                               Generator& aGenerator) {
  auto& player = GetPlayer();

  bool found{false};

  for (std::size_t level = 0u; level < mRevealed.size(); ++level) {
    for (std::size_t index = 0u; index < mRevealed[level].size(); ++index) {
      if (mRevealed[level][index] == aCard) {
        ReplaceCard(level, index, aGenerator);
        found = true;
        break;
      }
    }
  }
  if (!found) {
    auto const& reserved = player.GetReservedDevelopmentCards();
    for (std::size_t index = 0u; index < reserved.size(); ++index) {
      if (reserved[index] == aCard) {
        player.RemoveDevelopmentCard(index);
        found = true;
        break;
      }
    }
  }

  ASSERT(found);

  auto goldDemand = Gemset::GetGoldDemand(player.GetDiscount(),
                                          player.GetHeld(), aCard.GetCost());
  ASSERT(goldDemand <= player.GetGold());

  Gemset spend{};

  for (std::size_t i = 0u; i < kGemColorCount; ++i) {
    if (aCard.GetCost().Get(i) <= player.GetDiscount().Get(i)) {
      continue;
    }

    std::size_t deficit = aCard.GetCost().Get(i) - player.GetDiscount().Get(i);

    if (deficit <= player.GetHeld().Get(i)) {
      spend.Set(i, deficit);
    } else {
      spend.Set(i, player.GetHeld().Get(i));
    }
  }

  mAvailable = Gemset::Add(mAvailable, spend);
  mGold += goldDemand;
  ASSERT(mGold <= 5u);
  player.RemoveGems(spend);
  player.RemoveGold(goldDemand);

  player.AddDiscount(aCard.GetColor());
  player.AddPoints(aCard.GetPoints());
}

void GameState::DoReserveFaceUpMove(DevelopmentCard const& aCard,
                                    Generator& aGenerator) {
  for (std::size_t level = 0u; level < mRevealed.size(); ++level) {
    for (std::size_t index = 0u; index < mRevealed[level].size(); ++index) {
      if (mRevealed[level][index] == aCard) {
        ReplaceCard(level, index, aGenerator);
        bool revealed = true;
        DoReserveMove(aCard, revealed);
        return;
      }
    }
  }

  ASSERT_ALWAYS();
}

void GameState::DoReserveFaceDownMove(uint8 aLevel, Generator& aGenerator) {
  auto card = mDecks.Draw(aLevel, aGenerator);
  ASSERT(card);
  bool revealed = false;
  DoReserveMove(card, revealed);
}

void GameState::DoReserveMove(DevelopmentCard const& aCard, bool aRevealed) {
  GetPlayer().AddDevelopmentCard(aCard, aRevealed);
  if (mGold > 0) {
    GetPlayer().AddGold(1u);
    mGold--;
  }
}

void GameState::DoNobleMove(NobleCard const& aNoble) {
  for (auto& noble : mNobles) {
    if (noble == aNoble) {
      noble.Reset();
    }
  }

  GetPlayer().AddPoints(aNoble.GetPoints());

  ASSERT(Gemset::GetGoldDemand(GetPlayer().GetDiscount(), Gemset(),
                               aNoble.GetCost()) == 0);
}

void GameState::DoReturnMove(Gemset const& aGive) {
  for (std::size_t i = 0; i < kGemColorCount; ++i) {
    ASSERT(GetPlayer().GetHeld().Get(i) >= aGive.Get(i));
  }
  GetPlayer().RemoveGems(aGive);
  mAvailable = Gemset::Add(mAvailable, aGive);
}

DevelopmentCard GameState::ReplaceCard(uint8 aLevel, uint8 aIndex,
                                       Generator& aGenerator) {
  auto card = mRevealed[aLevel][aIndex];
  auto next = mDecks.Draw(aLevel, aGenerator);
  next.SetRevealed(true);
  if (next) {
    mRevealed[aLevel][aIndex] = next;
  } else {
    mRevealed[aLevel][aIndex].Reset();
  }
  return card;
}

void GameState::GetReturnMoves(std::vector<Move>& aMoves) const {
  std::size_t toReturnCount = GetPlayer().GetGemCount() - kMaxGemCount;
  ASSERT(toReturnCount > 0);

  std::size_t W = GetPlayer().GetHeld().Get(Color::kWhite);
  std::size_t U = GetPlayer().GetHeld().Get(Color::kBlue);
  std::size_t G = GetPlayer().GetHeld().Get(Color::kGreen);
  std::size_t R = GetPlayer().GetHeld().Get(Color::kRed);
  std::size_t B = GetPlayer().GetHeld().Get(Color::kBlack);

  for (std::size_t w = 0u; w <= std::min(W, toReturnCount); ++w) {
    for (std::size_t u = 0u; u <= std::min(U, toReturnCount - w); ++u) {
      for (std::size_t g = 0u; g <= std::min(G, toReturnCount - w - u); ++g) {
        for (std::size_t r = 0u; r <= std::min(R, toReturnCount - w - u - g);
             ++r) {
          for (std::size_t b = 0u;
               b <= std::min(B, toReturnCount - w - u - g - r); ++b) {
            if (w + u + g + r + b == toReturnCount) {
              aMoves.emplace_back(Move::MakeReturnMove(Gemset(w, u, g, r, b)));
            }
          }
        }
      }
    }
  }
}

std::size_t GameState::GetNobleMoves(std::vector<Move>* aMoves) const {
  std::size_t moveCount{0};
  std::size_t index{0};

  for (auto noble : mNobles) {
    if (noble) {
      std::size_t goldRequired = Gemset::GetGoldDemand(
          GetPlayer().GetDiscount(), Gemset(), noble.GetCost());
      if (goldRequired == 0) {
        if (aMoves) {
          aMoves->push_back(Move::MakeNobleMove(noble));
        }
        moveCount++;
      }
    }

    index++;
  }

  return moveCount;
}

void GameState::GetCollectMoves(std::vector<Move>& aMoves) const {
  std::size_t maxCollectCount{0u};
  for (size_t i = 0u; i < kGemColorCount; ++i) {
    if (mAvailable.Get(i) > 0) {
      maxCollectCount++;
    }
  }
  maxCollectCount = std::min(maxCollectCount, kMaxCollectCount);

  GetCollectMoves(aMoves, maxCollectCount);

  for (std::size_t i = 0u; i < kGemColorCount; ++i) {
    if (mAvailable.Get(i) >= 4) {
      Gemset take{};
      take.Set(i, 2u);
      aMoves.emplace_back(Move::MakeCollectMove(take));
    }
  }
}

void GameState::GetCollectMoves(std::vector<Move>& aMoves,
                                std::size_t aMaxCollectCount) const {
  std::size_t W = std::min(1ul, mAvailable.Get(Color::kWhite));
  std::size_t U = std::min(1ul, mAvailable.Get(Color::kBlue));
  std::size_t G = std::min(1ul, mAvailable.Get(Color::kGreen));
  std::size_t R = std::min(1ul, mAvailable.Get(Color::kRed));
  std::size_t B = std::min(1ul, mAvailable.Get(Color::kBlack));

  for (std::size_t w = 0u; w <= std::min(W, aMaxCollectCount); ++w) {
    for (std::size_t u = 0u; u <= std::min(U, aMaxCollectCount - w); ++u) {
      for (std::size_t g = 0u; g <= std::min(G, aMaxCollectCount - w - u);
           ++g) {
        for (std::size_t r = 0u; r <= std::min(R, aMaxCollectCount - w - u - g);
             ++r) {
          for (std::size_t b = 0u;
               b <= std::min(B, aMaxCollectCount - w - u - g - r); ++b) {
            if (w + u + g + r + b == aMaxCollectCount) {
              aMoves.emplace_back(Move::MakeCollectMove(Gemset(w, u, g, r, b)));
            }
          }
        }
      }
    }
  }
}

void GameState::GetPurchaseMoves(std::vector<Move>& aMoves) const {
  for (std::size_t level = 0u; level < mRevealed.size(); ++level) {
    for (std::size_t index = 0u; index < mRevealed[level].size(); ++index) {
      auto card = mRevealed[level][index];
      if (card) {
        TryAddPurchaseMove(aMoves, card);
      }
    }
  }

  auto const& reserved = GetPlayer().GetReservedDevelopmentCards();
  for (std::size_t index = 0u; index < reserved.size(); ++index) {
    auto card = reserved[index];
    if (card) {
      TryAddPurchaseMove(aMoves, card);
    }
  }
}

void GameState::TryAddPurchaseMove(std::vector<Move>& aMoves,
                                   DevelopmentCard const& aCard) const {
  std::size_t goldRequired = Gemset::GetGoldDemand(
      GetPlayer().GetDiscount(), GetPlayer().GetHeld(), aCard.GetCost());
  if (goldRequired > GetPlayer().GetGold()) {
    return;
  }

  aMoves.emplace_back(Move::MakePurchaseMove(aCard));
}

void GameState::GetReserveMoves(std::vector<Move>& aMoves) const {
  auto const& reserved = GetPlayer().GetReservedDevelopmentCards();
  if (reserved[0] && reserved[1] && reserved[2]) {
    // Player's hand is full.
    return;
  }

  for (std::size_t level = 0u; level < mRevealed.size(); ++level) {
    for (std::size_t index = 0u; index < mRevealed[level].size(); ++index) {
      auto card = mRevealed[level][index];
      if (card) {
        aMoves.emplace_back(Move::MakeReserveMove(card));
      }
    }
  }

  for (size_t i = 0u; i < kDevelopmentCardLevelCount; ++i) {
    if (mDecks.HasLevel(i) > 0) {
      aMoves.emplace_back(Move::MakeReserveMove(i));
    }
  }
}

// Set hidden info to plausible state.
void GameState::Determinize(Generator& aGenerator) {
  for (auto& player : mPlayers) {
    for (auto& slot : player.GetReservedDevelopmentCards()) {
      if (slot && slot.IsHidden()) {
        slot.ClearHidden(mDecks.Draw(slot.GetLevel(), aGenerator));
      }
    }
  }

  mDeterminized = true;
}

// Hide information not visible to provided player
GameState GameState::MaskHiddenInformation(uint8 aPlayer) {
  auto copy = *this;
  auto& otherPlayer = copy.mPlayers[1u - aPlayer];

  for (auto& slot : otherPlayer.GetReservedDevelopmentCards()) {
    if (slot && !slot.IsRevealed()) {
      auto card = slot.SetHidden();
      copy.mDecks.Insert(card);
    }
  }

  copy.mDeterminized = false;
  return copy;
}

bool GameState::HasHiddenInformation(uint8 aPlayer) const {
  auto& otherPlayer = mPlayers[1u - aPlayer];

  for (auto& slot : otherPlayer.GetReservedDevelopmentCards()) {
    if (slot && !slot.IsRevealed()) {
      return true;
    }
  }

  return false;
}

}  // namespace engine