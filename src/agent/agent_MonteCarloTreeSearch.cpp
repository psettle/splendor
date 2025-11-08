#include "agent_MonteCarloTreeSearch.hpp"

#include <iomanip>
#include <iostream>

#include "agent_Random.hpp"
#include "engine_GameState.hpp"
#include "util_Format.hpp"

namespace agent {

struct MonteCarloTreeSearch::Node {
  std::size_t mRolloutCount{};
  long mIntScore{};

  float GetScore() const { return mIntScore; }

  virtual ~Node() = default;
};

struct MonteCarloTreeSearch::MoveNode : public Node {
  Move mChosen{};
  std::vector<StateNode> mChildren{};
  std::size_t mAvailableCount{0u};

  MoveNode(engine::Move const& aMove) : mChosen(aMove) {}
};

class MonteCarloTreeSearch::MoveNodeSet {
 public:
  void Reserve(std::size_t aSize) { mStorage.reserve(aSize); }

  template <class Callable>
  void UpsertMoves(GameState const& aGamestate, Callable&& aCallable) {
    auto moves = aGamestate.GetMoves();
    mStorage.reserve(moves.size());

    for (auto const& newMove : moves) {
      UpsertMove(newMove);
    }

    for (auto const& newMove : moves) {
      auto& node = UpsertMove(newMove);
      aCallable(node);
    }
  }

 private:
  MoveNode& UpsertMove(Move const& aMove) {
    for (auto& move : mStorage) {
      if (move.mChosen == aMove) {
        return move;
      }
    }
    mStorage.emplace_back(aMove);
    return mStorage.back();
  }

  std::vector<MoveNode> mStorage;
};

struct MonteCarloTreeSearch::StateNode : public Node {
  GameState mState;

  StateNode(GameState const& aState) : mState(aState) {}

  std::vector<MoveNode*>& GetChildren() { return mChildren; }
  std::vector<MoveNode*>& GetUnexplored() { return mUnexplored; }
  GameState const& GetDeterminized() { return mDeterminized.value(); }

  void ResetRollout() {
    mChildren.clear();
    mUnexplored.clear();
    mDeterminized.reset();
  }
  void InitRollout(Generator& aGenerator) {
    ResetRollout();
    mDeterminized = mState;
    mDeterminized.value().Determinize(aGenerator);
    IterateMoves(
        [&](MoveNode& aNode) {
          aNode.mAvailableCount++;
          if (aNode.mRolloutCount == 0) {
            mUnexplored.emplace_back(&aNode);
          } else {
            mChildren.emplace_back(&aNode);
          }
        },
        aGenerator);
  }

 private:
  template <class Callable>
  void IterateMoves(Callable&& aCallable, Generator& aGenerator) {
    mMoveNodes.UpsertMoves(mDeterminized.value(), std::move(aCallable));
  }

  MoveNodeSet mMoveNodes{};
  std::vector<MoveNode*> mChildren{};
  std::vector<MoveNode*> mUnexplored{};
  std::optional<GameState> mDeterminized{};
};

MonteCarloTreeSearch::MonteCarloTreeSearch(Generator& aGenerator,
                                           Options const& aOptions)
    : mGenerator{aGenerator},
      mOptions{std::move(aOptions)},
      mRolloutAgent{aOptions.mMakeRolloutPolicy(aGenerator)} {
  mRunner.AddAgent(mRolloutAgent.get());
  mRunner.AddAgent(mRolloutAgent.get());
}

MonteCarloTreeSearch::~MonteCarloTreeSearch() {}

void MonteCarloTreeSearch::OnSetup(GameState const& aState, uint8 aPlayerId) {
  ResetHistory();
  mPlayerId = aPlayerId;
}

engine::Move MonteCarloTreeSearch::OnTurn(GameState const& aState) {
  TimeStamp start{};

  auto root = TrackActualAction(aState);
  StateNode storage{aState};
  if (!root) {
    root = &storage;
  }

  std::size_t maxPath{0};

  std::vector<Node*> expandPath;
  while (true) {
    expandPath.clear();
    root->InitRollout(mGenerator);
    expandPath.emplace_back(root);
    Select(expandPath);

    auto back = dynamic_cast<StateNode*>(expandPath.back());

    ASSERT(back);

    if (!back->GetUnexplored().empty()) {
      Expand(expandPath);
    }

    char score = Heuristic(*back);
    Backup(expandPath, score);

    maxPath = std::max(expandPath.size(), maxPath);

    if (CheckLimit(start)) {
      break;
    }
  }

  ASSERT(!root->GetChildren().empty());

  if (mOptions.mDebug) {
    std::sort(root->GetChildren().begin(), root->GetChildren().end(),
              [](MoveNode const* aLeft, MoveNode const* aRight) {
                return aLeft->GetScore() / aLeft->mRolloutCount >
                       aRight->GetScore() / aRight->mRolloutCount;
              });
    std::cout << "player " << static_cast<uint16>(mPlayerId + 1)
              << " rollouts: " << root->mRolloutCount
              << " strength: " << (root->GetScore() / root->mRolloutCount)
              << " depth: " << maxPath << std::endl;
    for (std::size_t i = 0; i < std::min(10ul, root->GetChildren().size());
         ++i) {
      auto const& move = root->GetChildren()[i]->mChosen;
      std::cout << "score:" << std::fixed << std::setprecision(5)
                << (root->GetChildren()[i]->GetScore() /
                    root->GetChildren()[i]->mRolloutCount)
                << '\t';
      std::cout << "sims:" << root->GetChildren()[i]->mRolloutCount << '\t';
      util::ShowMove(std::cout, mPlayerId, move);
      std::cout << '\n';
    }
    std::cout << "\n";
  }

  auto best =
      util::MaxElement(root->GetChildren().begin(), root->GetChildren().end(),
                       [](MoveNode const* aMove) {
                         ASSERT(aMove->mRolloutCount > 0);
                         return aMove->GetScore() / aMove->mRolloutCount;
                       });
  mPreviousMove = std::make_unique<MoveNode>(std::move(**best));
  return mPreviousMove->mChosen;
}

char MonteCarloTreeSearch::Heuristic(StateNode const& aLeaf) const {
  char score{0};
  for (std::size_t i = 0u; i < mOptions.mSimsPerRollout; ++i) {
    score += Simulate(aLeaf.mState);
  }
  return score;
}

void MonteCarloTreeSearch::ResetHistory() { mPreviousMove.reset(); }

MonteCarloTreeSearch::StateNode* MonteCarloTreeSearch::TrackActualAction(
    GameState const& aState) {
  if (!mPreviousMove) {
    return nullptr;
  }

  if (!mOptions.mTraceHistory) {
    return nullptr;
  }
  TimeStamp start{};

  for (auto& state : mPreviousMove->mChildren) {
    if (state.mState == aState) {
      if (mOptions.mDebug) {
        std::cout << "traced single: " << state.mRolloutCount << " rollouts in "
                  << (TimeStamp{} - start) << "s" << std::endl;
      }
      return &state;
    }
  }

  for (auto& state1 : mPreviousMove->mChildren) {
    state1.InitRollout(mGenerator);
    for (auto& move : state1.GetChildren()) {
      for (auto& state2 : move->mChildren) {
        if (state2.mState == aState) {
          if (mOptions.mDebug) {
            std::cout << "traced double: " << state2.mRolloutCount
                      << " rollouts in " << (TimeStamp{} - start) << "s"
                      << std::endl;
          }
          return &state2;
        }
      }
    }
  }

  return nullptr;
}

void MonteCarloTreeSearch::Select(std::vector<Node*>& aPath) {
  StateNode* back = dynamic_cast<StateNode*>(aPath.back());
  ASSERT(back);
  if (!back->GetUnexplored().empty()) {
    /* Unexplored actions on this path, we should explore them before going
     * deeper. */
    return;
  }

  if (back->GetChildren().empty()) {
    /* Terminal node (everything explored, no children), can't grow. */
    ASSERT(back->mState.IsTerminal());
    return;
  }

  ASSERT(back->mRolloutCount > 0);

  float factor = back->mState.GetNextPlayer() == mPlayerId ? 1.0 : -1.0;

  auto max = util::MaxElement(
      back->GetChildren().begin(), back->GetChildren().end(),
      [&](MoveNode const* aChild) {
        ASSERT(aChild->mRolloutCount > 0);
        float value = factor * aChild->GetScore() / aChild->mRolloutCount +
                      std::sqrt(mOptions.mUpperConfidenceBound *
                                std::log(aChild->mAvailableCount) /
                                aChild->mRolloutCount);
        return value;
      });

  /* Grow path and keep trying to find something to expand. */
  MoveNode* moveNode = *max;
  aPath.emplace_back(moveNode);

  StateNode* nextState = TraceMove(back->GetDeterminized(), moveNode);
  nextState->InitRollout(mGenerator);
  aPath.emplace_back(nextState);

  Select(aPath);
}

void MonteCarloTreeSearch::Expand(std::vector<Node*>& aPath) {
  StateNode* back = dynamic_cast<StateNode*>(aPath.back());
  ASSERT(back);

  auto& unexplored = back->GetUnexplored();
  auto it = unexplored.begin() + mGenerator() % unexplored.size();
  auto moveNode = *it;
  unexplored.erase(it);
  back->GetChildren().emplace_back(moveNode);
  auto stateNode = TraceMove(back->GetDeterminized(), moveNode);
  stateNode->InitRollout(mGenerator);

  aPath.push_back(moveNode);
  aPath.push_back(stateNode);
}

MonteCarloTreeSearch::StateNode* MonteCarloTreeSearch::TraceMove(
    GameState const& aStart, MoveNode* aMoveNode) {
  switch (aMoveNode->mChosen.mType) {
    case engine::MoveType::kCollect:
    case engine::MoveType::kReturn:
    case engine::MoveType::kNoble:
      // These moves are always deterministic.
      if (aMoveNode->mChildren.size() == 1) {
        return &aMoveNode->mChildren.back();
      }
      break;
    default:
      break;
  }

  // Otherwise the move results in a randomized state
  GameState aCopy = aStart;
  aCopy.DoMove(aMoveNode->mChosen, mGenerator);
  aCopy = aCopy.MaskHiddenInformation(mPlayerId);

  for (auto& existing : aMoveNode->mChildren) {
    if (existing.mState == aCopy) {
      return &existing;
    }
  }

  aMoveNode->mChildren.emplace_back(aCopy);
  return &aMoveNode->mChildren.back();
}

char MonteCarloTreeSearch::Simulate(GameState const& aState) const {
  GameState local = aState;
  local.Determinize(mGenerator);
  auto winner = mRunner.RunGame(local, mGenerator);
  return Score(winner);
}

char MonteCarloTreeSearch::Score(std::optional<uint8> aWinner) const {
  if (!aWinner) {
    return 0;
  }

  return (aWinner.value() == mPlayerId) ? 1 : -1;
}

void MonteCarloTreeSearch::Backup(std::vector<Node*> const& aPath,
                                  char aScore) const {
  for (auto node : aPath) {
    node->mRolloutCount += mOptions.mSimsPerRollout;
    node->mIntScore += aScore;
  }
}

}  // namespace agent