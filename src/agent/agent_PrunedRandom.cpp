#include "agent_PrunedRandom.hpp"

#include "engine_Move.hpp"

namespace agent {

engine::Move PrunedRandom::OnTurn(engine::GameState const& aState,
                                  std::vector<engine::Move>&& aMoves) {
  std::vector<engine::Move> purchase{};
  std::vector<engine::Move> collect{};
  purchase.reserve(aMoves.size());
  collect.reserve(aMoves.size());

  for (auto const& move : aMoves) {
    if (move.mType == engine::MoveType::kCollect) {
      collect.emplace_back(move);
    } else if (move.mType == engine::MoveType::kPurchase) {
      purchase.emplace_back(move);
    }
  }

  if (!purchase.empty()) {
    return purchase[mGenerator() % purchase.size()];
  }

  if (!collect.empty() && collect.front().mCollect.mTake.GetCount() > 0) {
    return collect[mGenerator() % collect.size()];
  }

  return aMoves[mGenerator() % aMoves.size()];
}

}  // namespace agent