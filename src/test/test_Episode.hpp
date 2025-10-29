#ifndef TEST_EPISODE_HPP
#define TEST_EPISODE_HPP

#include <optional>
#include <vector>

#include "engine_GameState.hpp"
#include "engine_Move.hpp"
#include "util_General.hpp"

namespace test {
struct Episode {
  struct Frame {
    engine::GameState mState;
    engine::Move mMove;
    uint8 mPlayer;
    std::optional<uint8> mWinner{};

    Frame(engine::GameState const& aState, engine::Move const& aMove,
          uint8 aPlayer)
        : mState(aState), mMove(aMove), mPlayer(aPlayer) {}
  };

  std::vector<Frame> mFrames;
};
}  // namespace test

#endif  // TEST_EPISODE_HPP