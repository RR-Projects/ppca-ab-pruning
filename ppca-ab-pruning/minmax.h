#pragma once

#include <map>
#include <list>
#include <algorithm>

#include "board.h"
#include "State.h"

using namespace std;
namespace ppca {

    class minmax
    {
    public:
        minmax();
        ~minmax();

        list<Board> GetTasks(Board& startingBoard, GAME_CHAR startPlayer, int minTaskCount, int maxDepth, int& searchedDepth);

        unsigned int ConvertToGpuData(int** data, const list<Board>& tasks);

        State GetBestMove(Board& startBoard, GAME_CHAR startPlayer, const map<unsigned int, long long>& gpuResults, int depth);
    };

}
