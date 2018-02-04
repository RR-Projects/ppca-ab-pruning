#include "main.h"

#include <thrust\host_vector.h>


#include <queue>
#include <cassert>

#include "board.h"
#include "PlayerLine.h"

void addTo(PlayerLine<5, 5, 4>& x, int el) {
	assert(x.ContinuesLine(el) == true);
	x.Add(el);
}

int _main() {

	Board _board(5, 5);

	int _continue = 0;
	size_t lastSize = 0;

	GAME_CHAR player = PLAYER;
	int depth = 11;
	queue<Board> res1, res2;
	res1.push(_board);
	while (depth > 0) {

		while (!res1.empty()) {
			Board& b = res1.front();

			thrust::host_vector<Board> childMoves;
			b.GenerateMoves(childMoves, player);
			for (int i = 0; i < childMoves.size(); i++) {
				res2.push(childMoves[i]);
			}

			res1.pop();

			if (res2.size() - lastSize > 10000000)
			{
				cout << "Size increased by 10 mil (" << res2.size() << "). Continue? ";
				cin >> _continue;
				if (_continue == 0)
					break;
				lastSize = res2.size();
			}
		}
		res1.swap(res2);

		cout << "Depth " << depth << " generated. " << endl
			<< "Board stats: (cr: " << Board::nCreated << ", cp: " << Board::nCopied << ", del:" << Board::nDeleted << ")" << endl
			<< res1.size() << " boards in queue ( aprox. " << res1.size() * 20 / 1024 / 1024 << " MB)." << endl
			<< "Continue? (0/1) ";

		cin >> _continue;

		if (_continue == 0)
			break;

		player = SWITCH_PLAYER(player);
		depth--;
	}

	cout << res1.size();
	cout << endl << "End. Press any key.";

	while (!res1.empty())
	{
		res1.pop();
	}

	return 0;
}