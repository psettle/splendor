#include "engine_Runner.hpp"

#include "engine_GameState.hpp"
#include "engine_IAgent.hpp"
#include "engine_IView.hpp"
#include "engine_Move.hpp"

namespace engine {

std::optional<uint8> Runner::RunGame(Generator& aGenerator) const {
  GameState state{aGenerator};
  return RunGame(state, aGenerator);
}

std::optional<uint8> Runner::RunGame(GameState& aState,
                                     Generator& aGenerator) const {
  ASSERT(mAgents.size() == 2u);

  for (size_t i = 0u; i < mAgents.size(); ++i) {
    mAgents[i]->OnSetup(aState, i);
  }

  while (!aState.IsTerminal()) {
    for (auto const& view : mViews) {
      view->ShowState(aState);
    }

    uint8 nextPlayer = aState.GetNextPlayer();
    auto agent = mAgents[nextPlayer];
    auto moves = aState.GetMoves();
    auto move = agent->OnTurn(aState, std::move(moves));

    for (auto const& view : mViews) {
      view->ShowTurn(aState, move, nextPlayer);
    }

    aState.DoMove(move, aGenerator);
  }

  return aState.GetWinner();
}

}  // namespace engine
