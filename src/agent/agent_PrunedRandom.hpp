#ifndef AGENT_PRUNEDRANDOM_HPP
#define AGENT_PRUNEDRANDOM_HPP

#include "engine_IAgent.hpp"
#include "util_General.hpp"

namespace agent {

/**
 * A splendor agent that makes random moves, but with a preference for purchase
 * moves, then collect moves, then any other move.
 */
class PrunedRandom : public engine::IAgent {
 public:
  using Generator = util::Generator;

  PrunedRandom(Generator& aGenerator) : mGenerator(aGenerator) {}

  void OnSetup(engine::GameState const& aState, uint8 aPlayerId) override {}

  engine::Move OnTurn(engine::GameState const& aState) override;

 private:
  Generator& mGenerator;
};

}  // namespace agent

#endif  // AGENT_PRUNEDRANDOM_HPP