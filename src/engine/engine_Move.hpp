#ifndef ENGINE_MOVE_HPP
#define ENGINE_MOVE_HPP

#include "engine_Gemset.hpp"
#include "util_General.hpp"

namespace engine {

enum class MoveType : uint8 {
  kCollect,
  kPurchase,
  kReserveFaceUp,
  kReserveFaceDown,
  kNoble,
  kReturn,
};

struct Move {
  MoveType mType;
  union {
    struct {
      Gemset mTake;
    } mCollect;
    struct {
      uint8 mLevel;  // 3 == HAND
      uint8 mIndex;
    } mPurchase;
    struct {
      uint8 mLevel;
      uint8 mIndex;
    } mReserveFaceUp;
    struct {
      uint8 mLevel;
    } mReserveFaceDown;
    struct {
      uint8 mIndex;
    } mNoble;
    struct {
      Gemset mGive;
    } mReturn;
  };

  static Move MakeReturnMove(Gemset aGive) {
    Move move{};
    move.mType = MoveType::kReturn;
    move.mReturn.mGive = aGive;
    return move;
  }

  static Move MakeNobleMove(uint8 aIndex) {
    Move move{};
    move.mType = MoveType::kNoble;
    move.mNoble.mIndex = aIndex;
    return move;
  }

  static Move MakeCollectMove(Gemset const& aTake) {
    Move move{};
    move.mType = MoveType::kCollect;
    move.mCollect.mTake = aTake;
    return move;
  }

  static Move MakePurchaseMove(uint8 aLevel, uint8 aIndex) {
    Move move{};
    move.mType = MoveType::kPurchase;
    move.mPurchase.mIndex = aIndex;
    move.mPurchase.mLevel = aLevel;
    return move;
  }

  static Move MakeReserveMove(uint8 aLevel) {
    Move move{};
    move.mType = MoveType::kReserveFaceDown;
    move.mReserveFaceDown.mLevel = aLevel;
    return move;
  }

  static Move MakeReserveMove(uint8 aLevel, uint8 aIndex) {
    Move move{};
    move.mType = MoveType::kReserveFaceUp;
    move.mReserveFaceUp.mIndex = aIndex;
    move.mReserveFaceUp.mLevel = aLevel;
    return move;
  }
};

}  // namespace engine

#endif  // ENGINE_MOVE_HPP