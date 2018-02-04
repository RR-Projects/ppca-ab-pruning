#pragma once

#include <iostream>
#include <thrust\host_vector.h>
#include <memory>

#include "ppca_helpers.h"

using namespace std;

class Board
{
public:
	static unsigned long long nCreated, nCopied, nDeleted;


	CUDA_CALLABLE_MEMBER Board();
	CUDA_CALLABLE_MEMBER Board(int x, int y);

	CUDA_CALLABLE_MEMBER Board(const Board& b);
	CUDA_CALLABLE_MEMBER Board(Board&& b);

	Board& operator=(const Board& b);

	CUDA_CALLABLE_MEMBER ~Board();

	void GenerateMoves(thrust::host_vector<Board>& result, GAME_CHAR player);

	unsigned int GetColumns();
	unsigned int GetRows();

	unsigned int inline GetCell(int x, int y) {
		return GetCellInternal(GetIndex(x, y), GetOffset(x, y));
	}

	void SetCell(int x, int y, unsigned int val);
	void Print();

private:
	unsigned int* data;
	unsigned int xy, filledCells, score;
	// min size of class: 16 bytes (4 * (1 int of data + 3 int properties))

	void UpdateScore(int x, int y);

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
