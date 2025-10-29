#include "agent_MonteCarloTreeSearch.hpp"
#include "agent_PrunedRandom.hpp"
#include "agent_Random.hpp"
#include "engine_Runner.hpp"
#include "test_Collect.hpp"
#include "test_Episode.hpp"
#include "test_IAgentFactory.hpp"
#include "view_Text.hpp"

class RandomFactory : public test::IAgentFactory {
 public:
  std::unique_ptr<engine::IAgent> MakeAgent(util::Generator& aGenerator) const {
    return std::make_unique<agent::Random>(aGenerator);
  }
};

class PrunedRandomFactory : public test::IAgentFactory {
 public:
  std::unique_ptr<engine::IAgent> MakeAgent(util::Generator& aGenerator) const {
    return std::make_unique<agent::PrunedRandom>(aGenerator);
  }
};

class MctsFactory : public test::IAgentFactory {
 public:
  std::unique_ptr<engine::IAgent> MakeAgent(util::Generator& aGenerator) const {
    return std::make_unique<agent::MonteCarloTreeSearch>(aGenerator, mOptions);
  }

  agent::MonteCarloTreeSearch::Options mOptions{};
};

#define TEST 0
#if TEST
int main() {
  MctsFactory mcts1{};
  MctsFactory mcts2{};

  mcts1.mOptions.mTimeoutSeconds = 1.0f;

  mcts2.mOptions.mTraceRandomMoves = true;
  mcts2.mOptions.mTimeoutSeconds = 1.0f;

  std::size_t mcts1Wins = 0u;
  std::size_t mcts2Wins = 0u;

  auto episodes = test::CollectEpisodes(mcts1, mcts2, 2048u, 14u);

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

  std::cout << "1: " << mcts1Wins << " 2: " << mcts2Wins << std::endl;
  return 0;
}
#else
int main() {
  auto generator = util::MakeGenerator();

  agent::MonteCarloTreeSearch::Options options{};
  options.mTimeoutSeconds = 60.0f;

  engine::Runner runner;
  agent::MonteCarloTreeSearch mcts1{generator, options};
  agent::MonteCarloTreeSearch mcts2{generator, options};
  view::Text textView{};

  runner.AddAgent(&mcts1);
  runner.AddAgent(&mcts2);
  runner.AddView(&textView);
  runner.RunGame(generator);

  return 0;
}
#endif  // TEST