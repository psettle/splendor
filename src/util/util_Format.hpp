#ifndef UTIL_FORMAT_HPP
#define UTIL_FORMAT_HPP

#include <iostream>
#include <optional>

#include "util_General.hpp"

namespace engine {
class GameState;
class Gemset;
class NobleCard;
class DevelopmentCard;
class Player;
class Move;
}  // namespace engine

namespace util {

void ShowState(std::ostream& aOut, engine::GameState const& aState);

void ShowGemset(std::ostream& aOut, engine::Gemset const& aSet,
                std::optional<uint8> aGold = std::nullopt);

void ShowCost(std::ostream& aOut, engine::Gemset const& aCost,
              std::optional<uint8> aGold = std::nullopt);

void ShowNoble(std::ostream& aOut, engine::NobleCard const& aNoble);

void ShowCard(std::ostream& aOut, engine::DevelopmentCard const& aCard,
              std::string const& aPrefix = "", bool aShowHidden = false);

void ShowPlayer(std::ostream& aOut, engine::Player const& aPlayer);

void ShowMove(std::ostream& aOut, uint8 aPlayer, engine::Move const& aMove);

}  // namespace util

#endif  // UTIL_FORMAT_HPP