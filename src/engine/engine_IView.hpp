#ifndef ENGINE_IVIEW_HPP
#define ENGINE_IVIEW_HPP

#include "util_General.hpp"

namespace engine {

class GameState;
class Move;

class IView {
 public:
  virtual ~IView() = default;

  virtual void ShowState(GameState const& aState) = 0;
  virtual void ShowTurn(GameState const& aState, Move const& aMove,
                        uint8 aPlayer) = 0;
};

}  // namespace engine

#endif  // ENGINE_IVIEW_HPP