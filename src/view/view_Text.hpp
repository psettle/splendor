#ifndef VIEW_TEXT_HPP
#define VIEW_TEXT_HPP

#include <iostream>
#include <optional>

#include "engine_IView.hpp"
#include "util_Format.hpp"

namespace view {

class Text : public engine::IView {
 public:
  Text(std::ostream& aOut = std::cout) : mOut(aOut) {}

  void ShowState(engine::GameState const& aState) override {
    ShowHeader("NOBLES");
    for (auto noble : aState.GetNobles()) {
      if (noble) {
        ShowNoble(noble);
      }
    }

    auto revealed = aState.GetRevealedDevelopmentCards();
    for (std::size_t l = 3; l > 0; --l) {
      ShowHeader("TIER " + std::to_string(l));
      for (std::size_t i = 0; i < 4; ++i) {
        auto card = revealed[l - 1][i];
        if (card) {
          ShowCard(card);
        }
      }
    }

    ShowHeader("AVAILABLE");
    util::ShowGemset(mOut, aState.GetAvailable(), aState.GetAvailableGold());
    mOut << "\n";

    std::size_t id = 0u;
    for (auto const& player : aState.GetPlayers()) {
      id++;
      ShowHeader("PLAYER " + std::to_string(id));
      ShowPlayer(player);
    }

    mOut << '\n';
  }

  void ShowTurn(engine::GameState const& aState, engine::Move const& aMove,
                uint8 aPlayer) override {
    ShowHeader("MOVE BY PLAYER " + std::to_string(aPlayer + 1));
    util::ShowMove(mOut, aState, aPlayer, aMove);
    mOut << "\n";
  }

 private:
  void ShowHeader(std::string const& aString) {
    mOut << "--- " << aString << " ---\n";
  }

  void ShowNoble(engine::NobleCard const& aNoble) {
    util::ShowNoble(mOut, aNoble);
    mOut << "\n";
  }

  void ShowCard(engine::DevelopmentCard const& aCard,
                std::string const& aPrefix = "") {
    util::ShowCard(mOut, aCard, aPrefix);
    mOut << "\n";
  }

  void ShowPlayer(engine::Player const& aPlayer) {
    util::ShowPlayer(mOut, aPlayer);
    mOut << "\n";
  }

  std::ostream& mOut;
};
}  // namespace view

#endif  // VIEW_TEXT_HPP