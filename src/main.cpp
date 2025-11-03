#include "agent_ConsolePlayer.hpp"
#include "agent_MonteCarloTreeSearch.hpp"
#include "agent_PrunedRandom.hpp"
#include "agent_Random.hpp"
#include "agent_SmartRollout.hpp"
#include "engine_Runner.hpp"
#include "test_Collect.hpp"
#include "test_Episode.hpp"
#include "test_IAgentFactory.hpp"
#include "view_Text.hpp"

class MctsFactory : public test::IAgentFactory {
 public:
  std::unique_ptr<engine::IAgent> MakeAgent(util::Generator& aGenerator) const {
    return std::make_unique<agent::MonteCarloTreeSearch>(aGenerator, mOptions);
  }

  agent::MonteCarloTreeSearch::Options mOptions{};
};

template <class Agent>
class SimpleFactory : public test::IAgentFactory {
 public:
  std::unique_ptr<engine::IAgent> MakeAgent(util::Generator& aGenerator) const {
    return std::make_unique<Agent>(aGenerator);
  }
};

template <class Agent>
class OptionsFactory : public test::IAgentFactory {
 public:
  std::unique_ptr<engine::IAgent> MakeAgent(util::Generator& aGenerator) const {
    return std::make_unique<Agent>(aGenerator, mOptions);
  }

  Agent::Options mOptions{};
};

#define TEST 0
#if TEST
int main() {
  MctsFactory mcts1{};
  MctsFactory mcts2{};

  float timeout = 1.0f;

  mcts1.mOptions.mTimeoutSeconds = timeout;
  mcts1.mOptions.mMakeRolloutPolicy =
      [](util::Generator& aGenerator) -> std::unique_ptr<engine::IAgent> {
    return std::make_unique<agent::Random>(aGenerator);
  };
  mcts1.mOptions.mUpperConfidenceBound = 2.0f;

  mcts2.mOptions.mTimeoutSeconds = timeout;
  mcts2.mOptions.mMakeRolloutPolicy =
      [](util::Generator& aGenerator) -> std::unique_ptr<engine::IAgent> {
    return std::make_unique<agent::SmartRollout>(aGenerator);
  };

  // SimpleFactory<agent::PrunedRandom> mcts1;
  // OptionsFactory<agent::SmartRollout> mcts2;
  // for (std::size_t i = 1u; i < 100; ++i) {
  std::size_t mcts1Wins = 0u;
  std::size_t mcts2Wins = 0u;

  mcts2.mOptions.mUpperConfidenceBound = 0.8f;

  auto episodes = test::CollectEpisodes(mcts1, mcts2, 256, 16);

  for (auto const& episode : episodes) {
    auto winner = episode.mFrames.front().mWinner;

    if (winner) {
      if (winner.value() == 0) {
        mcts1Wins++;
      } else {
        mcts2Wins++;
      }
    }
  }

  std::cout << " 1: " << mcts1Wins << " 2: " << mcts2Wins << std::endl;
  //}
  return 0;
}
#else
int main() {
  auto generator = util::MakeGenerator();

  agent::MonteCarloTreeSearch::Options options{};
  options.mTimeoutSeconds = 30.0f;
  options.mDebug = true;

  engine::Runner runner;
  agent::MonteCarloTreeSearch mcts1{generator, options};
  agent::MonteCarloTreeSearch mcts2{generator, options};
  view::Text textView{std::cout, false};

  runner.AddAgent(&mcts1);
  runner.AddAgent(&mcts2);
  runner.AddView(&textView);
  runner.RunGame(generator);

  return 0;
}
#endif  // TEST