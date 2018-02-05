#pragma once

#include <queue>
#include <map>
#include <stack>

#include "board.h"
#include "State.h"

using namespace std;
namespace ppca {

    class minmax
    {
    public:
        minmax();
        ~minmax();

        queue<Board> GetTasks(Board& startingBoard, GAME_CHAR startPlayer, int minTaskCount, int maxDepth, int& searchedDepth);

        Board GetBestMove(Board& startBoard, GAME_CHAR startPlayer, const map<unsigned int, long long int>& gpuResults, int depth);
    };

}
