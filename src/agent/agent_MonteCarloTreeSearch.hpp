#ifndef AGENT_MONTECARLOTREESEARCH_HPP
#define AGENT_MONTECARLOTREESEARCH_HPP

#include <limits>
#include <memory>
#include <optional>
#include <vector>

#include "agent_SmartRollout.hpp"
#include "engine_IAgent.hpp"
#include "engine_Runner.hpp"
#include "util_General.hpp"
#include "util_TimeStamp.hpp"

namespace engine {
class GameState;
class Move;
}  // namespace engine

namespace agent {

struct MonteCarloTreeSearchOptions {
  float mTimeoutSeconds{0.1f};
  float mUpperConfidenceBound{0.8f};
  bool mTraceHistory{true};
  std::unique_ptr<engine::IAgent> (*mMakeRolloutPolicy)(
      util::Generator& aGenerator) =
      [](util::Generator& aGenerator) -> std::unique_ptr<engine::IAgent> {
    return std::make_unique<agent::SmartRollout>(aGenerator);
  };
  bool mDebug{false};
  std::size_t mSimsPerRollout{5u};
};

class MonteCarloTreeSearch : public engine::IAgent {
 public:
  using Generator = util::Generator;
  using GameState = engine::GameState;
  using Move = engine::Move;
  using Options = MonteCarloTreeSearchOptions;

  MonteCarloTreeSearch(Generator& aGenerator,
                       Options const& aOptions = Options{});
  ~MonteCarloTreeSearch() override;

  void OnSetup(GameState const& aState, uint8 aPlayerId) override;
  Move OnTurn(GameState const& aState) override;

 private:
  using TimeStamp = util::TimeStamp;

  struct Node;
  struct MoveNode;
  struct StateNode;
  class MoveNodeSet;

  char Heuristic(StateNode const& aLeaf) const;

  bool CheckLimit(TimeStamp const& aStart) {
    return aStart.Since() >= mOptions.mTimeoutSeconds;
  }

  void ResetHistory();

  StateNode* TrackActualAction(GameState const& aState);
  void Select(std::vector<Node*>& aPath);
  void Expand(std::vector<Node*>& aPath);
  StateNode* TraceMove(GameState const& aStart, MoveNode* aMoveNode);
  char Simulate(GameState const& aState) const;
  char Score(std::optional<uint8> aWinner) const;
  void Backup(std::vector<Node*> const& aPath, char aScore) const;

  std::unique_ptr<MoveNode> mPreviousMove{};
  uint8 mPlayerId{};
  engine::Runner mRunner{};
  Generator& mGenerator;
  Options mOptions;
  std::unique_ptr<engine::IAgent> mRolloutAgent{};
};
}  // namespace agent

#endif /* AGENT_MONTECARLOTREESEARCH_HPP */
