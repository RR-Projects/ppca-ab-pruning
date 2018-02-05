#include "main.h"

#include <queue>

#include "board.h"
#include "minmax.h"

int _main() {
	Board _board(3, 3, 3);
	ppca::minmax mmAlg;

	GAME_CHAR player = PLAYER;

	int depth = _board.GetColumns() * _board.GetRows();

	int blockSize;      // The launch configurator returned block size 
	int minGridSize;    // The minimum grid size needed to achieve the maximum occupancy for a full device launch 
	int gridSize;       // The actual grid size needed, based on input size



	auto tasks = mmAlg.GetTasks(_board, player, 1000, depth);

	return 0;
}