#ifndef ENGINE_MOVE_HPP
#define ENGINE_MOVE_HPP

#include "engine_DevelopmentCard.hpp"
#include "engine_Gemset.hpp"
#include "engine_NobleCard.hpp"
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
      DevelopmentCard mCard;
    } mPurchase;
    struct {
      DevelopmentCard mCard;
    } mReserveFaceUp;
    struct {
      uint8 mLevel;
    } mReserveFaceDown;
    struct {
      NobleCard mNoble;
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

  static Move MakeNobleMove(NobleCard const& aNoble) {
    Move move{};
    move.mType = MoveType::kNoble;
    move.mNoble.mNoble = aNoble;
    return move;
  }

  static Move MakeCollectMove(Gemset const& aTake) {
    Move move{};
    move.mType = MoveType::kCollect;
    move.mCollect.mTake = aTake;
    return move;
  }

  static Move MakePurchaseMove(DevelopmentCard const& aCard) {
    Move move{};
    move.mType = MoveType::kPurchase;
    move.mPurchase.mCard = aCard;
    move.mPurchase.mCard.SetRevealed(true);
    return move;
  }

  static Move MakeReserveMove(uint8 aLevel) {
    Move move{};
    move.mType = MoveType::kReserveFaceDown;
    move.mReserveFaceDown.mLevel = aLevel;
    return move;
  }

  static Move MakeReserveMove(DevelopmentCard const& aCard) {
    Move move{};
    move.mType = MoveType::kReserveFaceUp;
    move.mReserveFaceUp.mCard = aCard;
    move.mReserveFaceUp.mCard.SetRevealed(true);
    return move;
  }

  bool operator==(Move const& aOther) const {
    if (mType != aOther.mType) {
      return false;
    }

    switch (mType) {
      case MoveType::kCollect:
        return mCollect.mTake == aOther.mCollect.mTake;
      case MoveType::kPurchase:
        return mPurchase.mCard == aOther.mPurchase.mCard;
      case MoveType::kReserveFaceUp:
        return mReserveFaceUp.mCard == aOther.mReserveFaceUp.mCard;
      case MoveType::kReserveFaceDown:
        return mReserveFaceDown.mLevel == aOther.mReserveFaceDown.mLevel;
      case MoveType::kNoble:
        return mNoble.mNoble == aOther.mNoble.mNoble;
      case MoveType::kReturn:
        return mReturn.mGive == aOther.mReturn.mGive;
      default:
        return true;
    }
  }

  bool operator<(Move const& aOther) const {
    if (mType < aOther.mType) {
      return true;
    }

    switch (mType) {
      case MoveType::kCollect:
        return mCollect.mTake < aOther.mCollect.mTake;
      case MoveType::kPurchase:
        return mPurchase.mCard < aOther.mPurchase.mCard;
      case MoveType::kReserveFaceUp:
        return mReserveFaceUp.mCard < aOther.mReserveFaceUp.mCard;
      case MoveType::kReserveFaceDown:
        return mReserveFaceDown.mLevel < aOther.mReserveFaceDown.mLevel;
      case MoveType::kNoble:
        return mNoble.mNoble < aOther.mNoble.mNoble;
      case MoveType::kReturn:
        return mReturn.mGive < aOther.mReturn.mGive;
      default:
        return false;
    }
  }

  bool operator>(Move const& aOther) const {
    return !(*this < aOther) && !(*this == aOther);
  }
};

}  // namespace engine

#endif  // ENGINE_MOVE_HPP