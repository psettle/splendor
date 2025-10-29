#ifndef ENGINE_IAGENT_HPP
#define ENGINE_IAGENT_HPP

#include <vector>

#include "util_General.hpp"

namespace engine {

class GameState;
class Move;

class IAgent {
 public:
  virtual ~IAgent() = default;

  virtual void OnSetup(GameState const& aState, uint8 aPlayerId) = 0;
  virtual Move OnTurn(GameState const& aState, std::vector<Move>&& aMoves) = 0;
};

}  // namespace engine

#endif  // ENGINE_IAGENT_HPP