#pragma once

#include <queue>

#include "board.h"

using namespace std;
namespace ppca {

	class minmax
	{
	public:
		minmax();
		~minmax();

		queue<Board> GetTasks(const Board& startingBoard, GAME_CHAR startPlayer, int minTaskCount, int maxDepth);

		Board& GetBestMove(const queue<Board>& gpuResults, GAME_CHAR startPlayer);
	};

}
