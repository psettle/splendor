#include "agent_MonteCarloTreeSearch.hpp"

#include <iomanip>
#include <iostream>

#include "agent_Random.hpp"
#include "engine_GameState.hpp"
#include "util_Format.hpp"

namespace agent {

struct MonteCarloTreeSearch::Node {
  std::size_t mRolloutCount{};
  float mScore{};

  virtual ~Node() = default;
};

struct MonteCarloTreeSearch::MoveNode : public Node {
  Move mChosen{};
  std::vector<StateNode> mChildren{};

  MoveNode(engine::Move const& aMove) : mChosen(aMove) {}
};

struct MonteCarloTreeSearch::StateNode : public Node {
  GameState mState;
  std::vector<MoveNode> mChildren{};
  std::vector<Move> mUnexplored{};

  StateNode(GameState const& aState, std::vector<Move>&& aMoves)
      : mState(aState), mUnexplored(std::move(aMoves)) {}
};

MonteCarloTreeSearch::MonteCarloTreeSearch(Generator& aGenerator,
                                           Options const& aOptions)
    : mGenerator{aGenerator},
      mOptions{std::move(aOptions)},
      mRolloutAgent{aGenerator} {
  mRunner.AddAgent(&mRolloutAgent);
  mRunner.AddAgent(&mRolloutAgent);
}

MonteCarloTreeSearch::~MonteCarloTreeSearch() {}

void MonteCarloTreeSearch::OnSetup(GameState const& aState, uint8 aPlayerId) {
  ResetHistory();
  mPlayerId = aPlayerId;
}

engine::Move MonteCarloTreeSearch::OnTurn(GameState const& aState,
                                          std::vector<Move>&& aMoves) {
  TimeStamp start{};

  auto root = TrackActualAction(aState);
  StateNode storage{aState, std::move(aMoves)};
  if (!root) {
    root = &storage;
  }

  std::size_t maxPath{0};

  std::vector<Node*> expandPath;
  while (true) {
    expandPath.clear();
    expandPath.emplace_back(root);
    Select(expandPath);

    auto back = dynamic_cast<StateNode*>(expandPath.back());

    ASSERT(back);

    if (!back->mUnexplored.empty()) {
      Expand(expandPath);
    }

    float mScore = Heuristic(*back);

    Backup(expandPath, mScore);

    maxPath = std::max(expandPath.size(), maxPath);

    if (CheckLimit(start)) {
      break;
    }
  }

  ASSERT(!root->mChildren.empty());

  static bool constexpr debug = true;
  if constexpr (debug) {
    std::sort(root->mChildren.begin(), root->mChildren.end(),
              [](MoveNode const& left, MoveNode const& right) {
                return left.mScore / left.mRolloutCount >
                       right.mScore / right.mRolloutCount;
              });
  }

  auto best = util::MaxElement(root->mChildren.begin(), root->mChildren.end(),
                               [](MoveNode const& n) {
                                 ASSERT(n.mRolloutCount > 0);
                                 return n.mScore / n.mRolloutCount;
                               });

  if constexpr (debug) {
    std::cout << "player " << static_cast<uint16>(mPlayerId + 1)
              << " rollouts: " << root->mRolloutCount
              << " strength: " << (root->mScore / root->mRolloutCount)
              << " depth: " << maxPath << std::endl;
    for (std::size_t i = 0; i < std::min(5ul, root->mChildren.size()); ++i) {
      auto const& move = root->mChildren[i].mChosen;
      std::cout << "score:" << std::fixed << std::setprecision(5)
                << (root->mChildren[i].mScore /
                    root->mChildren[i].mRolloutCount)
                << '\t';
      util::ShowMove(std::cout, root->mState, mPlayerId, move);
      std::cout << '\n';
    }
    std::cout << "\n";
  }

  mPreviousMove = std::make_unique<MoveNode>(std::move(*best));
  return mPreviousMove->mChosen;
}

float MonteCarloTreeSearch::Heuristic(StateNode const& aLeaf) const {
  return Simulate(aLeaf.mState);
}

void MonteCarloTreeSearch::ResetHistory() { mPreviousMove.reset(); }

MonteCarloTreeSearch::StateNode* MonteCarloTreeSearch::TrackActualAction(
    GameState const& aState) {
  if (!mPreviousMove) {
    return nullptr;
  }

  if (!mOptions.mTraceRandomMoves && mPreviousMove->mChildren.size() != 1u) {
    // If the move has multiple outcomes, don't examine it
    return nullptr;
  }

  for (auto& state : mPreviousMove->mChildren) {
    if (state.mState == aState) {
      return &state;
    }
  }

  for (auto& state1 : mPreviousMove->mChildren) {
    for (auto& move : state1.mChildren) {
      if (!mOptions.mTraceRandomMoves && move.mChildren.size() != 1u) {
        // If the move has multiple outcomes, don't examine it.
        continue;
      }

      for (auto& state2 : move.mChildren) {
        if (state2.mState == aState) {
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
  if (!back->mUnexplored.empty()) {
    /* Unexplored actions on this path, we should explore them before going
     * deeper. */
    return;
  }

  if (back->mChildren.empty()) {
    /* Terminal node (everything explored, no children), can't grow. */
    ASSERT(back->mState.IsTerminal());
    return;
  }

  ASSERT(back->mRolloutCount > 0);

  float factor = back->mState.GetNextPlayer() == mPlayerId ? 1.0 : -1.0;

  auto max = util::MaxElement(
      back->mChildren.begin(), back->mChildren.end(),
      [&](MoveNode const& child) {
        ASSERT(child.mRolloutCount > 0);
        float value =
            factor * child.mScore / child.mRolloutCount +
            std::sqrt(mOptions.mUpperConfidenceBound *
                      std::log(back->mRolloutCount) / child.mRolloutCount);
        return value;
      });

  /* Grow path and keep trying to find something to expand. */
  MoveNode* moveNode = &*max;
  aPath.emplace_back(moveNode);

  StateNode* nextState = TraceMove(back->mState, moveNode);
  aPath.emplace_back(nextState);

  Select(aPath);
}

void MonteCarloTreeSearch::Expand(std::vector<Node*>& aPath) {
  StateNode* back = dynamic_cast<StateNode*>(aPath.back());
  ASSERT(back);

  auto& unexplored = back->mUnexplored;
  auto it = unexplored.begin() + mGenerator() % unexplored.size();
  auto move = *it;
  unexplored.erase(it);

  back->mChildren.emplace_back(move);

  auto moveNode = &back->mChildren.back();
  auto stateNode = TraceMove(back->mState, moveNode);

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
    case engine::MoveType::kPurchase:
      if (aMoveNode->mChosen.mPurchase.mLevel == 3) {
        // Purchase from hand is deterministic
        if (aMoveNode->mChildren.size() == 1) {
          return &aMoveNode->mChildren.back();
        }
      }
      break;
    default:
      break;
  }

  if (aMoveNode->mChildren.size() >= mOptions.mMaxNextStates) {
    return &aMoveNode->mChildren[mGenerator() % aMoveNode->mChildren.size()];
  }

  // Otherwise move results in randomized state

  GameState aCopy = aStart;
  aCopy.DoMove(aMoveNode->mChosen, mGenerator);

  for (auto& existing : aMoveNode->mChildren) {
    if (existing.mState == aCopy) {
      return &existing;
    }
  }

  aMoveNode->mChildren.emplace_back(aCopy, aCopy.GetMoves());
  return &aMoveNode->mChildren.back();
}

float MonteCarloTreeSearch::Simulate(GameState const& aState) const {
  GameState local = aState;
  auto winner = mRunner.RunGame(local, mGenerator);
  return Score(winner);
}

float MonteCarloTreeSearch::Score(std::optional<uint8> aWinner) const {
  if (!aWinner) {
    return 0.0f;
  }

  return (aWinner.value() == mPlayerId) ? 1.0f : -1.0f;
}

void MonteCarloTreeSearch::Backup(std::vector<Node*> const& aPath,
                                  float aScore) {
  for (auto it = aPath.rbegin(); it != aPath.rend(); ++it) {
    (*it)->mRolloutCount++;
    (*it)->mScore += aScore;
  }
}

}  // namespace agent