#include "engine_GameState.hpp"

#include <algorithm>

#include "engine_Move.hpp"

namespace engine {

GameState::GameState(Generator& aGenerator) {
  for (std::size_t level = 0; level < mRevealed.size(); ++level) {
    for (std::size_t index = 0; index < mRevealed[level].size(); ++index) {
      mRevealed[level][index] = mDecks.Draw(level, aGenerator);
    }
  }

  mNobles = NobleCard::ShuffleNobles(aGenerator);
  mNextPlayer = aGenerator() % mPlayers.size();
}

std::vector<Move> GameState::GetMoves() const {
  if (IsTerminal()) {
    return std::vector<Move>();
  }

  std::vector<Move> moves{};
  moves.reserve(60u);

  if (GetPlayer().GetPhase() == Player::TurnPhase::kReturn) {
    GetReturnMoves(moves);
    ASSERT(moves.size() > 0);
  } else if (GetPlayer().GetPhase() == Player::TurnPhase::kNoble) {
    GetNobleMoves(moves);
    ASSERT(moves.size() > 0);
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
  switch (aMove.mType) {
    case MoveType::kCollect:
      DoCollectMove(aMove.mCollect.mTake);
      break;
    case MoveType::kPurchase:
      DoPurchaseMove(aMove.mPurchase.mLevel, aMove.mPurchase.mIndex,
                     aGenerator);
      break;
    case MoveType::kReserveFaceUp:
      DoReserveFaceUpMove(aMove.mReserveFaceUp.mLevel,
                          aMove.mReserveFaceUp.mIndex, aGenerator);
      break;
    case MoveType::kReserveFaceDown:
      DoReserveFaceDownMove(aMove.mReserveFaceDown.mLevel, aGenerator);
      break;
    case MoveType::kNoble:
      DoNobleMove(aMove.mNoble.mIndex);
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

void GameState::DoPurchaseMove(uint8 aLevel, uint8 aIndex,
                               Generator& aGenerator) {
  auto& player = GetPlayer();
  DevelopmentCard card{};
  if (aLevel >= kDevelopmentCardLevelCount) {
    card = player.RemoveDevelopmentCard(aIndex);
  } else {
    card = ReplaceCard(aLevel, aIndex, aGenerator);
  }

  ASSERT(card);

  auto goldDemand = Gemset::GetGoldDemand(player.GetDiscount(),
                                          player.GetHeld(), card.GetCost());
  ASSERT(goldDemand <= player.GetGold());

  Gemset spend{};

  for (std::size_t i = 0u; i < kGemColorCount; ++i) {
    if (card.GetCost().Get(i) <= player.GetDiscount().Get(i)) {
      continue;
    }

    std::size_t deficit = card.GetCost().Get(i) - player.GetDiscount().Get(i);

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

  player.AddDiscount(card.GetColor());
  player.AddPoints(card.GetPoints());
}

void GameState::DoReserveFaceUpMove(uint8 aLevel, uint8 aIndex,
                                    Generator& aGenerator) {
  auto card = ReplaceCard(aLevel, aIndex, aGenerator);
  ASSERT(card);
  DoReserveMove(card);
}

void GameState::DoReserveFaceDownMove(uint8 aLevel, Generator& aGenerator) {
  auto card = mDecks.Draw(aLevel, aGenerator);
  ASSERT(card);
  DoReserveMove(card);
}

void GameState::DoReserveMove(DevelopmentCard aCard) {
  GetPlayer().AddDevelopmentCard(aCard);
  if (mGold > 0) {
    GetPlayer().AddGold(1u);
    mGold--;
  }
}

void GameState::DoNobleMove(uint8 aIndex) {
  auto noble = mNobles[aIndex];
  ASSERT(noble);
  mNobles[aIndex].Reset();

  GetPlayer().AddPoints(noble.GetPoints());

  ASSERT(Gemset::GetGoldDemand(GetPlayer().GetDiscount(), Gemset(),
                               noble.GetCost()) == 0);
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
  GetReturnMoves(aMoves, 0u, toReturnCount, Gemset());
}

void GameState::GetReturnMoves(std::vector<Move>& aMoves,
                               std::size_t aColorIndex,
                               std::size_t aToReturnCount,
                               Gemset aCurrent) const {
  if (aCurrent.GetCount() == aToReturnCount) {
    // No room left in set
    aMoves.emplace_back(Move::MakeReturnMove(aCurrent));
    return;
  }

  if (aColorIndex >= kGemColorCount) {
    // No colors left.
    return;
  }

  // Explore options where we return the current color
  std::size_t limit = std::min(aToReturnCount - aCurrent.GetCount(),
                               GetPlayer().GetHeld().Get(aColorIndex));
  for (std::size_t i = 0u; i <= limit; ++i) {
    aCurrent.Set(aColorIndex, i);
    GetReturnMoves(aMoves, aColorIndex + 1u, aToReturnCount, aCurrent);
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
          aMoves->push_back(Move::MakeNobleMove(index));
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
        TryAddPurchaseMove(aMoves, level, index, card);
      }
    }
  }

  auto const& reserved = GetPlayer().GetReservedDevelopmentCards();
  for (std::size_t index = 0u; index < reserved.size(); ++index) {
    auto card = reserved[index];
    if (card) {
      TryAddPurchaseMove(aMoves, 3u, index, card);
    }
  }
}

void GameState::TryAddPurchaseMove(std::vector<Move>& aMoves, uint8 aLevel,
                                   uint8 aIndex,
                                   DevelopmentCard const& aCard) const {
  std::size_t goldRequired = Gemset::GetGoldDemand(
      GetPlayer().GetDiscount(), GetPlayer().GetHeld(), aCard.GetCost());
  if (goldRequired > GetPlayer().GetGold()) {
    return;
  }

  aMoves.emplace_back(Move::MakePurchaseMove(aLevel, aIndex));
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
        aMoves.emplace_back(Move::MakeReserveMove(level, index));
      }
    }
  }

  for (size_t i = 0u; i < kDevelopmentCardLevelCount; ++i) {
    if (mDecks.HasLevel(i) > 0) {
      aMoves.emplace_back(Move::MakeReserveMove(i));
    }
  }
}

}  // namespace engine