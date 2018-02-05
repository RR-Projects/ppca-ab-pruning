#include "minmax.h"

namespace ppca {

	minmax::minmax() { }

	queue<Board> minmax::GetTasks(const Board& startBoard, GAME_CHAR startPlayer, int minTaskCount, int maxDepth) {

		queue<Board> res1, res2;
		res1.push(startBoard);

		while (true) {
			while (!res1.empty()) {
				Board& b = res1.front();

				Board* childMoves = nullptr;
				auto moveCount = b.GenerateMoves(&childMoves, startPlayer);
				for (int i = 0; i < moveCount; i++) {
					res2.push(childMoves[i]);
				}

				res1.pop();
				delete[] childMoves;
			}

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

	Board& minmax::GetBestMove(const queue<Board>& gpuResults, GAME_CHAR startPlayer) {

		return Board();
	}

	minmax::~minmax() { }
}
