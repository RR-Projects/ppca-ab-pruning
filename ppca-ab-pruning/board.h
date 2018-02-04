#pragma once

#include <iostream>
#include <memory>

#include "ppca_helpers.h"
#include "PlayerLine.h"

using namespace std;

class Board
{
public:
	static unsigned long long nCreated, nCopied, nDeleted;

	CUDA_CALLABLE_MEMBER Board();
	CUDA_CALLABLE_MEMBER Board(int x, int y, int lineLength);

	CUDA_CALLABLE_MEMBER Board(const Board& b);
	CUDA_CALLABLE_MEMBER Board(Board&& b);

	Board& operator=(const Board& b);

	CUDA_CALLABLE_MEMBER ~Board();

	int GenerateMoves(Board** results, GAME_CHAR player);

	unsigned int GetColumns();
	unsigned int GetRows();

	unsigned int inline GetCell(int x, int y) {
		return GetCellInternal(GetIndex(x, y), GetOffset(x, y));
	}

	void SetCell(int x, int y, unsigned int val);
	void Print();

	void CalculateScore();
	long long int GetScore() { return score; }
private:
	unsigned int* data;
	unsigned int xy, filledCells, lineLength;
	long long int score;
	// min size of class: 16 bytes (4 * (1 int of data + 3 int properties))

	PlayerLine** scoreLines;
	int scoreLinesSize;

	void CalculateScoreOnDirection(Point slowIncrement, Point fastIncrement, bool startFromTopRight = false, bool applyInitialSlowIncrement = false);

	unsigned int inline GetCellInternal(int index, char offset) {
		return (data[index] >> offset) & 0x3;
	}

	unsigned int inline GetIndex(int x, int y) {
		return this->pos(x, y) / 32;
	}

	unsigned int inline GetOffset(int x, int y) {
		return this->pos(x, y) % 32;
	}

	unsigned int pos(int x, int y) {
		return (GetColumns() * y + x) << 1;
	}

	unsigned int inline GetSize() {
		return ((GetColumns() * GetRows() << 1) + 31) / (sizeof(int) << 3); // (x * y * 2 + 7) / (8 * sizeof(int)); 
	}
};
