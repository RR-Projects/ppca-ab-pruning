#pragma once

#include <list>

#include "board.h"

using namespace std;

class minmax
{
public:
	minmax();
	~minmax();

	list<Board> GetTasks(Board* startingBoard, int minTaskCount, int depth = 12);
};

