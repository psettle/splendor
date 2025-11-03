#ifndef AGENT_CONSOLEPLAYER_HPP
#define AGENT_CONSOLEPLAYER_HPP

#include <iostream>

#include "engine_GameState.hpp"
#include "engine_IAgent.hpp"
#include "engine_Move.hpp"
#include "util_Format.hpp"
#include "util_General.hpp"

namespace agent {

class ConsolePlayer : public engine::IAgent {
 public:
  void OnSetup(engine::GameState const& aState, uint8 aPlayerId) override {
    mId = aPlayerId;
  }
  engine::Move OnTurn(engine::GameState const& aState) override {
    util::ShowState(std::cout, aState);
    auto moves = aState.GetMoves();

    for (std::size_t i = 0u; i < moves.size(); ++i) {
      std::cout << i << ": ";
      util::ShowMove(std::cout, mId, moves[i]);
      std::cout << "\n";
    }

    std::cout << "You are player " << static_cast<uint16>(mId + 1u)
              << std::endl;
    std::cout << "Entry move choice #:" << std::endl;
    do {
      std::cout << "Entry move choice #:" << std::endl;
      std::size_t choice;
      std::cin >> choice;

      if (choice < moves.size()) {
        return moves[choice];
      }
    } while (true);
  }

 private:
  uint8 mId{};
};

}  // namespace agent

#endif  // AGENT_CONSOLEPLAYER_HPP