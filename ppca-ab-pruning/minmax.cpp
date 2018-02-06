#include "minmax.h"

namespace ppca {

    minmax::minmax() { }

    list<Board> minmax::GetTasks(Board& startBoard, GAME_CHAR startPlayer, int minTaskCount, int maxDepth, int& searchedDepth) {

        list<Board> res1, res2;

        searchedDepth = 0;
        startBoard.SetTreePosition(0);
        res1.push_back(startBoard);
        maxDepth--;

        while (true) {
            while (!res1.empty()) {
                Board& b = res1.front();

                if (b.IsTerminal()) {
                    res2.push_back(b);
                    res1.pop_front();
                    continue;
                }

                Board* childMoves = nullptr;
                auto moveCount = b.GenerateMoves(&childMoves, startPlayer);
                for (int i = 0; i < moveCount; i++) {
                    res2.push_back(childMoves[i]);
                }


                res1.pop_front();
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

    unsigned int minmax::ConvertToGpuData(int** data, const list<Board>& tasks)
    {
        unsigned int size = tasks.front().GetSizeForBuffer() * tasks.size();

        auto start = *data = new int[size];

        for each (const Board& b in tasks)
        {
            start = b.CopyToBuffer(start);
        }

        return size;
    }

    State minmax::GetBestMove(Board& startBoard, GAME_CHAR startPlayer, const map<unsigned int, long long>& gpuResults, int depth) {
        int maxDepth = depth;
        GAME_CHAR player = startPlayer;

        startBoard.SetTreePosition(0);
        list<State> _stack;
        _stack.push_back(State(player, startBoard));

        State bestMove(player);

        while (!_stack.empty()) {
            State* parent = &_stack.back();

            if (depth <= 0 || !parent->GetBoard().HasNextMove() || parent->GetBoard().IsTerminal()) {
                State* child = new State(*parent); // copy constructor
                _stack.pop_back();

                if (_stack.empty()) // reached top of tree
                    continue;

                player = SWITCH_PLAYER(player);
                depth++;
                parent = &_stack.back();

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

                parent->SetAlpha(
                    CHAR_IS(player, PLAYER) * max(score, parent->GetAlpha()) +  // MAX
                    CHAR_IS(player, OPPONENT) * parent->GetAlpha() // MIN
                );

                parent->SetBeta(
                    CHAR_IS(player, PLAYER) * parent->GetBeta() + // MAX
                    CHAR_IS(player, OPPONENT) * min(score, parent->GetBeta())  // MIN
                );

                // alpha beta pruning
                if (parent->GetAlpha() > parent->GetBeta()) {

                    _stack.pop_back();
                    player = SWITCH_PLAYER(player);
                    depth++;
                }
            }
            else if (parent->GetBoard().HasNextMove()) {
                State move(SWITCH_PLAYER(player));
                parent->GetBoard().GetNextMove(move.GetBoard(), player);
                move.SetAlpha(parent->GetAlpha());
                move.SetBeta(parent->GetBeta());

                _stack.push_back(move);
                player = SWITCH_PLAYER(player);
                depth--;
            }

            // TODO: check alpha beta pruning
        }


        return bestMove;
    }

    minmax::~minmax() { }
}
