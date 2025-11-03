#ifndef AGENT_RANDOM_HPP
#define AGENT_RANDOM_HPP

#include "engine_GameState.hpp"
#include "engine_IAgent.hpp"
#include "engine_Move.hpp"
#include "util_General.hpp"

namespace agent {

/**
 * A splendor agent that makes random moves.
 */
class Random : public engine::IAgent {
 public:
  using Generator = util::Generator;

  Random(Generator& aGenerator) : mGenerator(aGenerator) {}

  void OnSetup(engine::GameState const& aState, uint8 aPlayerId) override {}

  engine::Move OnTurn(engine::GameState const& aState) override {
    auto state = aState;
    state.Determinize(mGenerator);
    auto moves = state.GetMoves();
    return moves[mGenerator() % moves.size()];
  }

 private:
  Generator& mGenerator;
};

}  // namespace agent

#endif  // AGENT_RANDOM_HPP