#include "test_Collect.hpp"

#include <iostream>
#include <memory>

#include "engine_IAgent.hpp"
#include "engine_IView.hpp"
#include "engine_Runner.hpp"
#include "pthread.h"
#include "test_Episode.hpp"
#include "test_IAgentFactory.hpp"

namespace test {

class EpisodeObserver : public engine::IView {
 public:
  EpisodeObserver(Episode& aEpisode) : mEpisode(aEpisode) {}

  void ShowState(engine::GameState const& aState) override {}

  void ShowTurn(engine::GameState const& aState, engine::Move const& aMove,
                uint8 aPlayer) override {
    mEpisode.mFrames.emplace_back(aState, aMove, aPlayer);
  };

 private:
  Episode& mEpisode;
};

struct TestControl {
  IAgentFactory const* mLeft;
  IAgentFactory const* mRight;
  std::size_t mGameCount;
  pthread_t mThread{};
  std::size_t mIndex;
  pthread_mutex_t* mMutex;
  std::vector<Episode>* mAllEpisodes;
};

static Episode CollectEpisode(IAgentFactory const& aLeft,
                              IAgentFactory const& aRight) {
  auto generator = util::MakeGenerator();
  auto leftAgent = aLeft.MakeAgent(generator);
  auto rightAgent = aRight.MakeAgent(generator);

  Episode episode{};
  episode.mFrames.reserve(150);

  EpisodeObserver observer{episode};

  engine::Runner runner{};
  runner.AddAgent(leftAgent.get());
  runner.AddAgent(rightAgent.get());
  runner.AddView(&observer);

  auto winner = runner.RunGame(generator);

  for (auto& frame : episode.mFrames) {
    frame.mWinner = winner;
  }

  return episode;
}

static void* RunTestsThread(void* aUserData) {
  TestControl& testControl = *static_cast<TestControl*>(aUserData);

  for (std::size_t i = 0u; i < testControl.mGameCount; ++i) {
    auto result = CollectEpisode(*testControl.mLeft, *testControl.mRight);

    pthread_mutex_lock(testControl.mMutex);
    testControl.mAllEpisodes->emplace_back(std::move(result));
    // std::cout << ".";
    // std::cout.flush();
    pthread_mutex_unlock(testControl.mMutex);
  }

  return nullptr;
}

std::vector<Episode> CollectEpisodes(IAgentFactory const& aLeft,
                                     IAgentFactory const& aRight,
                                     std::size_t aSampleCount,
                                     std::size_t aThreadCount) {
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, nullptr);

  std::vector<Episode> episodes;

  std::vector<TestControl> tests(aThreadCount);

  for (std::size_t i = 0u; i < aThreadCount; ++i) {
    auto& test = tests[i];
    test.mLeft = &aLeft;
    test.mRight = &aRight;
    test.mGameCount = aSampleCount / aThreadCount;
    test.mIndex = i;
    test.mMutex = &mutex;
    test.mAllEpisodes = &episodes;
    pthread_create(&test.mThread, NULL, RunTestsThread, &test);
  }

  for (auto& test : tests) {
    pthread_join(test.mThread, NULL);
  }

  pthread_mutex_destroy(&mutex);

  return episodes;
}

}  // namespace test