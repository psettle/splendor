#ifndef VIEW_TEXT_HPP
#define VIEW_TEXT_HPP

#include <iostream>
#include <optional>

#include "engine_IView.hpp"
#include "util_Format.hpp"

namespace view {

class Text : public engine::IView {
 public:
  Text(std::ostream& aOut = std::cout, bool aFair = false)
      : mOut(aOut), mFair(aFair) {}

  void ShowState(engine::GameState const& aState) override {
    if (!mFair) {
      util::ShowState(mOut, aState);
    }
  }

  void ShowTurn(engine::GameState const& aState, engine::Move const& aMove,
                uint8 aPlayer) override {
    ShowHeader("MOVE BY PLAYER " + std::to_string(aPlayer + 1));
    util::ShowMove(mOut, aPlayer, aMove);
    mOut << "\n";
  }

 private:
  void ShowHeader(std::string const& aString) {
    mOut << "--- " << aString << " ---\n";
  }

  std::ostream& mOut;
  bool mFair;
};
}  // namespace view

#endif  // VIEW_TEXT_HPP