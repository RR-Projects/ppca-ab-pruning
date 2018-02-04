#pragma once

#include <algorithm>

#include "Point.h"

#define NEXT_IN_BOUNDS(current, maxBound) std::min(current + 1, maxBound)

class PlayerLine
{
public:
	PlayerLine(int boardX, int maxLineLength, int startPos) : boardX(boardX), i(maxLineLength), j(maxLineLength) {
		gridPositions = new int[(maxLineLength << 1) + 1];
		gridPositions[i] = startPos;
	}

	void Add(int pos) {
		int t = (pos < gridPositions[i]) ? (i - 1) : (j + 1);
		gridPositions[t] = pos;
		i -= pos < gridPositions[i];
		j += pos > gridPositions[j];
	}

	bool ContinuesLine(int pos) {
		Point posPoint(pos, boardX);
		Point minPoint(gridPositions[i], boardX);
		Point maxPoint(gridPositions[j], boardX);

		auto isAdjacent = posPoint.isAdjacent(minPoint) || posPoint.isAdjacent(maxPoint);

		auto minDirection = posPoint - minPoint;
		auto maxDirection = maxPoint - posPoint;

		auto lineDir = Direction();

		return isAdjacent && (lineDir.isZero() || lineDir == minDirection || lineDir == maxDirection);
	}

	int inline Size() { return j - i + 1; }

	~PlayerLine() {
		delete[] gridPositions;
	}
private:

	int* gridPositions;
	int i, j, boardX;

	Point Direction() {
		Point r;
		r.x = (gridPositions[i] % boardX) - (gridPositions[NEXT_IN_BOUNDS(i, j)] % boardX);
		r.y = (gridPositions[i] / boardX) - (gridPositions[NEXT_IN_BOUNDS(i, j)] / boardX);
		return r;
	}
};
