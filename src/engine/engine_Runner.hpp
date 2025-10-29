#ifndef ENGINE_RUNNER_HPP
#define ENGINE_RUNNER_HPP

#include <optional>
#include <vector>

#include "util_General.hpp"

namespace engine {

class IAgent;
class IView;
class GameState;

class Runner {
 public:
  using Generator = util::Generator;

  void AddAgent(IAgent* aAgent) { mAgents.push_back(aAgent); }
  void AddView(IView* aView) { mViews.push_back(aView); }

  std::optional<uint8> RunGame() const {
    auto generator = util::MakeGenerator();
    return RunGame(generator);
  }

  std::optional<uint8> RunGame(Generator& aGenerator) const;
  std::optional<uint8> RunGame(GameState& aState, Generator& aGenerator) const;

 private:
  std::vector<IAgent*> mAgents{};
  std::vector<IView*> mViews{};
};

}  // namespace engine

#endif  // ENGINE_RUNNER_HPP