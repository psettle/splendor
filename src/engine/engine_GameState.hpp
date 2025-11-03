#ifndef ENGINE_GAMESTATE_HPP
#define ENGINE_GAMESTATE_HPP

#include <array>
#include <cstring>
#include <optional>
#include <vector>

#include "engine_DevelopmentCard.hpp"
#include "engine_Gemset.hpp"
#include "engine_NobleCard.hpp"
#include "engine_Player.hpp"
#include "util_General.hpp"

namespace engine {

class Move;

class GameState {
 public:
  using Generator = util::Generator;

  GameState(Generator& aGenerator);

  uint8 GetNextPlayer() const { return mNextPlayer; }
  std::vector<Move> GetMoves() const;
  auto GetNobles() const { return mNobles; }
  auto GetRevealedDevelopmentCards() const { return mRevealed; }
  std::optional<uint8> GetWinner() const;
  auto const& GetPlayers() const { return mPlayers; }
  Gemset const& GetAvailable() const { return mAvailable; }
  uint8 GetAvailableGold() const { return mGold; }

  bool IsTerminal() const;
  void DoMove(Move const& aMove, Generator& aGenerator);

  bool operator==(GameState const& aOther) const {
    ASSERT(mDeterminized == aOther.mDeterminized);

    return 0 == memcmp(this, &aOther, sizeof(*this));
  }

  GameState MaskHiddenInformation() {
    return MaskHiddenInformation(mNextPlayer);
  }
  GameState MaskHiddenInformation(uint8 aPlayer);
  void Determinize(Generator& aGenerator);

  bool HasHiddenInformation(uint8 aPlayer) const;

 private:
  static std::size_t constexpr kDevelopmentCardRevealCount = 4u;
  static std::size_t constexpr kWinningPointCount = 15u;
  static std::size_t constexpr kMaxTurnCount = 254;
  static std::size_t constexpr kMaxGemCount = 10u;
  static std::size_t constexpr kMaxCollectCount = 3u;

  void DoCollectMove(Gemset const& aTake);
  void DoPurchaseMove(DevelopmentCard const& aCard, Generator& aGenerator);
  void DoReserveFaceUpMove(DevelopmentCard const& aCard, Generator& aGenerator);
  void DoReserveFaceDownMove(uint8 aLevel, Generator& aGenerator);
  void DoReserveMove(DevelopmentCard const& aCard, bool aRevealed);
  void DoNobleMove(NobleCard const& aNoble);
  void DoReturnMove(Gemset const& aGive);

  DevelopmentCard ReplaceCard(uint8 aLevel, uint8 aIndex,
                              Generator& aGenerator);

  void GetReturnMoves(std::vector<Move>& aMoves) const;
  void GetNobleMoves(std::vector<Move>& aMoves) const {
    GetNobleMoves(&aMoves);
  }
  std::size_t GetNobleMoves(std::vector<Move>* aMoves) const;
  bool HasNobleMoves() const { return GetNobleMoves(nullptr) > 0; }
  void GetCollectMoves(std::vector<Move>& aMoves) const;
  void GetCollectMoves(std::vector<Move>& aMoves,
                       std::size_t aMaxCollectCount) const;
  void GetPurchaseMoves(std::vector<Move>& aMoves) const;
  void TryAddPurchaseMove(std::vector<Move>& aMoves,
                          DevelopmentCard const& aCard) const;
  void GetReserveMoves(std::vector<Move>& aMoves) const;

  Player const& GetPlayer() const { return mPlayers[mNextPlayer]; }
  Player& GetPlayer() { return mPlayers[mNextPlayer]; }

  Decks mDecks{};
  std::array<Player, 2u> mPlayers{};
  using RevealedRow = std::array<DevelopmentCard, kDevelopmentCardRevealCount>;
  std::array<RevealedRow, kDevelopmentCardLevelCount> mRevealed;
  std::array<NobleCard, NobleCard::kRevealedNobleCount> mNobles;
  Gemset mAvailable{4u};
  uint8 mGold{5u};
  uint8 mNextPlayer;
  bool mDeterminized{true};
};

}  // namespace engine

#endif  // ENGINE_GAMESTATE_HPP