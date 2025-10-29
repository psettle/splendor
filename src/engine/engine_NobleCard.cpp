#include "engine_NobleCard.hpp"

#include <array>

namespace engine {

std::array<NobleCard::Internal, 10> const NobleCard::kAllNobleCards = {
    NobleCard::Internal(Gemset(4, 4, 0, 0, 0)),
    NobleCard::Internal(Gemset(0, 4, 4, 0, 0)),
    NobleCard::Internal(Gemset(0, 0, 4, 4, 0)),
    NobleCard::Internal(Gemset(0, 0, 0, 4, 4)),
    NobleCard::Internal(Gemset(4, 0, 0, 0, 4)),
    NobleCard::Internal(Gemset(3, 3, 3, 0, 0)),
    NobleCard::Internal(Gemset(0, 3, 3, 3, 0)),
    NobleCard::Internal(Gemset(0, 0, 3, 3, 3)),
    NobleCard::Internal(Gemset(3, 0, 0, 3, 3)),
    NobleCard::Internal(Gemset(3, 3, 0, 0, 3)),
};

std::array<NobleCard, NobleCard::kRevealedNobleCount> NobleCard::ShuffleNobles(
    util::Generator& aGenerator) {
  std::array<uint8, kAllNobleCards.size()> allNobles{};

  for (std::size_t i = 0u; i < allNobles.size(); ++i) {
    allNobles[i] = i;
  }

  std::shuffle(allNobles.begin(), allNobles.end(), aGenerator);

  std::array<NobleCard, kRevealedNobleCount> nobles{};
  for (std::size_t i = 0; i < 3; ++i) {
    nobles[i] = allNobles[i];
  }

  return nobles;
}

}  // namespace engine