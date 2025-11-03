#ifndef AGENT_SMARTROLLOUT_HPP
#define AGENT_SMARTROLLOUT_HPP

#include "engine_IAgent.hpp"

namespace engine {
class Gemset;
}

namespace agent {

struct SmartRolloutOptions {
  uint8 mNearTermCostThreshold{3u};
  uint8 mPurchaseForDevelopmentCardWeight{2u};
  uint8 mPurchaseForNobleCardWeight{0u};
  uint8 mPurchaseForPointsWeight{100u};
};

class SmartRollout : public engine::IAgent {
 public:
  using Generator = util::Generator;
  using Options = SmartRolloutOptions;

  SmartRollout(Generator& aGenerator, Options const& aOptions = Options())
      : mGenerator(aGenerator), mOptions(aOptions) {}

  void OnSetup(engine::GameState const& aState, uint8 aPlayerId) override {}
  engine::Move OnTurn(engine::GameState const& aState);

 private:
  engine::Gemset GetNobleCost(engine::GameState const& aState) const;
  engine::Gemset GetCardCost(engine::GameState const& aState) const;
  engine::Move SelectCollectMove(std::vector<engine::Move> const& aMoves,
                                 engine::Gemset const& aCardCost);
  engine::Move SelectPurchaseMove(std::vector<engine::Move> const& aMoves,
                                  engine::Gemset const& aCardCost,
                                  engine::Gemset const& aNobleCost);

  Generator& mGenerator;
  Options mOptions;
};

}  // namespace agent

#endif  // AGENT_SMARTROLLOUT_HPP