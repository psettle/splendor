#ifndef AGENT_MONTECARLOTREESEARCH_HPP
#define AGENT_MONTECARLOTREESEARCH_HPP

#include <limits>
#include <memory>
#include <optional>
#include <vector>

#include "agent_PrunedRandom.hpp"
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
  std::size_t mMaxNextStates{std::numeric_limits<std::size_t>::max()};
  float mTimeoutSeconds{0.1f};
  float mUpperConfidenceBound{2.0f};
  bool mTraceRandomMoves{false};
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
  Move OnTurn(GameState const& aState, std::vector<Move>&& aMoves) override;

 private:
  using TimeStamp = util::TimeStamp;

  struct Node;
  struct MoveNode;
  struct StateNode;

  float Heuristic(StateNode const& aLeaf) const;

  bool CheckLimit(TimeStamp const& aStart) {
    return aStart.Since() >= mOptions.mTimeoutSeconds;
  }

  void ResetHistory();

  StateNode* TrackActualAction(GameState const& aState);
  void Select(std::vector<Node*>& aPath);
  void Expand(std::vector<Node*>& aPath);
  StateNode* TraceMove(GameState const& aStart, MoveNode* aMoveNode);
  float Simulate(GameState const& aState) const;
  float Score(std::optional<uint8> aWinner) const;
  static void Backup(std::vector<Node*> const& aPath, float aScore);

  std::unique_ptr<MoveNode> mPreviousMove{};
  uint8 mPlayerId{};
  engine::Runner mRunner{};
  Generator& mGenerator;
  Options mOptions;
  agent::PrunedRandom mRolloutAgent;
};
}  // namespace agent

#endif /* AGENT_MONTECARLOTREESEARCH_HPP */
