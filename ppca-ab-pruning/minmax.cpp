#include "minmax.h"

namespace ppca {

    minmax::minmax() { }

    queue<Board> minmax::GetTasks(Board& startBoard, GAME_CHAR startPlayer, int minTaskCount, int maxDepth, int& searchedDepth) {

        queue<Board> res1, res2;

        searchedDepth = 0;
        startBoard.SetTreePosition(0);
        res1.push(startBoard);
        maxDepth--;

        while (true) {
            while (!res1.empty()) {
                Board& b = res1.front();

                if (b.IsTerminal()) {
                    res2.push(b);
                    res1.pop();
                    continue;
                }

                Board* childMoves = nullptr;
                auto moveCount = b.GenerateMoves(&childMoves, startPlayer);
                for (int i = 0; i < moveCount; i++) {
                    res2.push(childMoves[i]);
                }


                res1.pop();
                delete[] childMoves;
            }
            searchedDepth++;

            if (res2.size() >= minTaskCount)
                return res2;

            if (maxDepth <= 0)
                break;

            // move to res1 and do another loop
            res1.swap(res2);
            startPlayer = SWITCH_PLAYER(startPlayer);
            maxDepth--;
        }

        return res2;
    }

    Board minmax::GetBestMove(Board& startBoard, GAME_CHAR startPlayer, const map<unsigned int, long long int>& gpuResults, int depth) {
        int maxDepth = depth;
        GAME_CHAR player = startPlayer;

        startBoard.SetTreePosition(0);
        stack<State> _stack;
        _stack.push(State(player, startBoard));

        State bestMove(player);

        while (!_stack.empty()) {
            State* parent = &_stack.top();

            if (depth <= 0 || !parent->GetBoard().HasNextMove() || parent->GetBoard().IsTerminal()) {
                State* child = new State(*parent); // copy constructor
                _stack.pop();

                if (_stack.empty()) // reached top of tree
                    continue;

                player = SWITCH_PLAYER(player);
                depth++;
                parent = &_stack.top();

                auto score = child->GetScore();
                if (gpuResults.find(child->GetBoard().GetTreePosition()) != gpuResults.end())
                    score = gpuResults.at(child->GetBoard().GetTreePosition());

                auto prevScore = parent->GetScore();

                // TODO: maybe move in the !HasNextMove part
                if (depth == maxDepth &&
                    (CHAR_IS(player, PLAYER) && prevScore < score ||
                        CHAR_IS(player, OPPONENT) && prevScore > score))
                    bestMove = *child;

                parent->SetScore(
                    CHAR_IS(player, PLAYER) * max(score, prevScore) + // MAX
                    CHAR_IS(player, OPPONENT) * min(score, prevScore)  // MIN
                );
            }
            else if (parent->GetBoard().HasNextMove()) {
                State move(SWITCH_PLAYER(player));
                parent->GetBoard().GetNextMove(move.GetBoard(), player);
                _stack.push(move);
                player = SWITCH_PLAYER(player);
                depth--;
            }

            // TODO: check alpha beta pruning
        }


        return bestMove.GetBoard();
    }

    minmax::~minmax() { }
}
