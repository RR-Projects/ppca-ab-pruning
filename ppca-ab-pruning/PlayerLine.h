#pragma once

#include <algorithm>

#include "Point.h"

#define GET_NEXT_INDEX(_array, comparer, i, j) ((i - 1) & (comparer < _array[i])) | ((j + 1) & (comparer > _array[j]))
#define NEXT_IN_BOUNDS(current, maxBound) std::min(current + 1, maxBound)

template<int BoardX, int BoardY, int MaxLineLength>
class PlayerLine
{
public:
	PlayerLine(int startPos) {
		gridPositions = new int[(MaxLineLength << 1) + 1];
		i = j = MaxLineLength;
		gridPositions[i] = startPos;
	}

	void Add(int pos) {
		int t = (pos < gridPositions[i]) ? (i - 1) : (j + 1);
		gridPositions[t] = pos;
		//gridPositions[GET_NEXT_INDEX(gridPositions, pos, i, j)] = pos;
		i -= pos < gridPositions[i];
		j += pos > gridPositions[j];
	}

	bool ContinuesLine(int pos) {
		Point posPoint(pos, BoardX);
		Point minPoint(gridPositions[i], BoardX);
		Point maxPoint(gridPositions[j], BoardX);

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
	int i, j;

	Point Direction() {
		Point r;
		r.x = (gridPositions[i] % BoardX) - (gridPositions[NEXT_IN_BOUNDS(i, j)] % BoardX);
		r.y = (gridPositions[i] / BoardX) - (gridPositions[NEXT_IN_BOUNDS(i, j)] / BoardX);
		return r;
	}
};
