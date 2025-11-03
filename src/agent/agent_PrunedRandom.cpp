#include "agent_PrunedRandom.hpp"

#include "engine_GameState.hpp"
#include "engine_Move.hpp"

namespace agent {

engine::Move PrunedRandom::OnTurn(engine::GameState const& aState) {
  auto state = aState;
  state.Determinize(mGenerator);
  auto moves = state.GetMoves();
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

  if (!purchase.empty()) {
    return purchase[mGenerator() % purchase.size()];
  }

  if (!collect.empty() && collect.front().mCollect.mTake.GetCount() > 0) {
    return collect[mGenerator() % collect.size()];
  }

  return moves[mGenerator() % moves.size()];
}

}  // namespace agent