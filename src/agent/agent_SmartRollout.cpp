
#include "agent_SmartRollout.hpp"

#include "engine_GameState.hpp"
#include "engine_Move.hpp"
#include "util_General.hpp"

namespace agent {

engine::Move SmartRollout::OnTurn(engine::GameState const& aState) {
  auto state = aState;
  state.Determinize(mGenerator);
  auto moves = state.GetMoves();

  engine::Gemset cardCosts = GetCardCost(state);
  engine::Gemset nobleCosts = GetNobleCost(state);

  std::vector<engine::Move> purchase{};
  std::vector<engine::Move> collect{};
  purchase.reserve(moves.size());
  collect.reserve(moves.size());

  for (auto const& move : moves) {
    if (move.mType == engine::MoveType::kCollect) {
      collect.emplace_back(move);
    } else if (move.mType == engine::MoveType::kPurchase) {
      purchase.emplace_back(move);
    }
  }

  auto const& player = aState.GetPlayers()[aState.GetNextPlayer()];
  if (!collect.empty() && player.GetHeld().GetCount() <= 7 &&
      collect.front().mCollect.mTake.GetCount() == 3) {
    return SelectCollectMove(collect, cardCosts);
  }

  if (!purchase.empty()) {
    return SelectPurchaseMove(purchase, cardCosts, nobleCosts);
  }

  if (!collect.empty() && collect.front().mCollect.mTake.GetCount() > 0) {
    return SelectCollectMove(collect, cardCosts);
  }

  return moves[mGenerator() % moves.size()];
}

engine::Gemset SmartRollout::GetNobleCost(
    engine::GameState const& aState) const {
  engine::Gemset nobleCosts{};
  for (auto const& noble : aState.GetNobles()) {
    if (noble) {
      nobleCosts = engine::Gemset::Add(nobleCosts, noble.GetCost());
    }
  }
  return nobleCosts;
}

engine::Gemset SmartRollout::GetCardCost(
    engine::GameState const& aState) const {
  auto const& player = aState.GetPlayers()[aState.GetNextPlayer()];
  auto purchasePower =
      engine::Gemset::Add(player.GetDiscount(), player.GetHeld());

  // Get costs for cards we are close to purchasing (missing <= 6 resources)
  engine::Gemset cardCosts{};
  auto addCosts = [&](engine::DevelopmentCard const& aCard) {
    if (!aCard) {
      return;
    }

    auto discountCost =
        engine::Gemset::ApplyDiscount(aCard.GetCost(), purchasePower);

    if (discountCost.GetCount() + player.GetGold() <=
        mOptions.mNearTermCostThreshold) {
      cardCosts = engine::Gemset::Add(cardCosts, discountCost);
    }
  };

  for (auto const& row : aState.GetRevealedDevelopmentCards()) {
    for (auto const& card : row) {
      addCosts(card);
    }
  }
  for (auto const& card : player.GetReservedDevelopmentCards()) {
    addCosts(card);
  }

  return cardCosts;
}

engine::Move SmartRollout::SelectCollectMove(
    std::vector<engine::Move> const& aMoves, engine::Gemset const& aCardCost) {
  return util::WeightedSample(
      aMoves,
      [&](std::size_t aIndex) {
        std::size_t weight{1ul};
        auto const& take = aMoves[aIndex].mCollect.mTake;
        for (std::size_t i = 0u; i < engine::kGemColorCount; ++i) {
          weight += take.Get(i) * aCardCost.Get(i);
        }
        return weight;
      },
      mGenerator);
}

engine::Move SmartRollout::SelectPurchaseMove(
    std::vector<engine::Move> const& aMoves, engine::Gemset const& aCardCost,
    engine::Gemset const& aNobleCost) {
  return util::WeightedSample(
      aMoves,
      [&](std::size_t aIndex) {
        auto const& card = aMoves[aIndex].mPurchase.mCard;

        size_t weight{1ul};

        weight += aCardCost.Get(card.GetColor()) *
                  mOptions.mPurchaseForDevelopmentCardWeight;
        weight += aNobleCost.Get(card.GetColor()) *
                  mOptions.mPurchaseForNobleCardWeight;
        weight += card.GetPoints() * mOptions.mPurchaseForPointsWeight;

        return weight;
      },
      mGenerator);
}

}  // namespace agent
