#include "board.h"

unsigned long long Board::nCreated = 0ULL;
unsigned long long Board::nCopied = 0ULL;
unsigned long long Board::nDeleted = 0ULL;

CUDA_CALLABLE_MEMBER Board::Board() : data(nullptr), xy(0), score(0), filledCells(0), lineLength(0) { }
CUDA_CALLABLE_MEMBER Board::Board(int x, int y, int lineLength) : filledCells(0), data(nullptr), lineLength(lineLength)
{
	nCreated++;
	xy = (x << 16) | (y & 0xFFFF);

	score = x * y;
	auto size = GetSize();
	data = new unsigned int[size];
	memset(data, 0, size * sizeof(int));

	//scoreLines
}

CUDA_CALLABLE_MEMBER Board::Board(const Board& b) {
	operator=(b);
}

CUDA_CALLABLE_MEMBER Board::Board(Board&& b) : xy(b.xy), filledCells(b.filledCells), score(b.score), lineLength(b.lineLength) {
	data = b.data;
}

Board& Board::operator=(const Board & b)
{
	nCopied++;

	lineLength = b.lineLength;
	xy = b.xy;
	filledCells = b.filledCells;
	score = b.score;

	auto size = GetSize();
	data = new unsigned int[size];
	memcpy(data, b.data, size * sizeof(int));

	return *this;
}

int Board::GenerateMoves(Board** results, GAME_CHAR player) {
#ifdef DEBUG
	if (*results != nullptr) {
		cout << "A pointer to an NULL array required.";
		throw 1;
	}
#endif

	*results = new Board[GetColumns() * GetRows() - filledCells];
	auto _results = *results;

	int idx = 0;

	for (unsigned int j = 0; j < GetRows(); j++) {
		for (unsigned int i = 0; i < GetColumns(); i++) {

			if (GetCell(i, j) == 0) {
				_results[idx] = *this;
				_results[idx].SetCell(i, j, player);
				idx++;
			}
		}
	}

	return idx;
}

void Board::SetCell(int x, int y, unsigned int val) {
	int index = GetIndex(x, y);
	char offset = GetOffset(x, y);

	auto exVal = GetCellInternal(index, offset);

	auto exValBit = (((exVal + 1) >> 1) & 0x1);
	auto valBit = (((val + 1) >> 1) & 0x1);
	filledCells += (-1 * exValBit) | (valBit ^ exValBit);

	val = (val & CELL_BITMAP) << offset; // first two bits shifted by offset amount
	unsigned int clearPos = ~(CELL_BITMAP << offset);

	data[index] = (data[index] & clearPos) | val;
}

void Board::CalculateScore() {
	score = GetRows() * GetColumns() - filledCells; // reset

	Point slowIncrement, fastIncrement;

	// horizontal move
	slowIncrement.x = 0;
	slowIncrement.y = 1;
	fastIncrement.x = 1;
	fastIncrement.y = 0;
	CalculateScoreOnDirection(slowIncrement, fastIncrement);

	// vertical move
	slowIncrement.x = 1;
	slowIncrement.y = 0;
	fastIncrement.x = 0;
	fastIncrement.y = 1;
	CalculateScoreOnDirection(slowIncrement, fastIncrement);

	// first diagonal, upper part
	slowIncrement.x = 1;
	slowIncrement.y = 0;
	fastIncrement.x = 1;
	fastIncrement.y = 1;
	CalculateScoreOnDirection(slowIncrement, fastIncrement, false);

	// first diagonal, lower part
	slowIncrement.x = 0;
	slowIncrement.y = 1;
	fastIncrement.x = 1;
	fastIncrement.y = 1;
	CalculateScoreOnDirection(slowIncrement, fastIncrement, false, true);

	// second diagonal, upper part
	slowIncrement.x = 1;
	slowIncrement.y = 0;
	fastIncrement.x = -1;
	fastIncrement.y = 1;
	CalculateScoreOnDirection(slowIncrement, fastIncrement);

	// second diagonal, lower part
	slowIncrement.x = 0;
	slowIncrement.y = 1;
	fastIncrement.x = -1;
	fastIncrement.y = 1;
	CalculateScoreOnDirection(slowIncrement, fastIncrement, true, true);
}

void Board::CalculateScoreOnDirection(Point slowIncrement, Point fastIncrement, bool startFromTopRight, bool applyInitialSlowIncrement) {
	int rows = GetRows(), columns = GetColumns();
	unsigned int i = 0 + applyInitialSlowIncrement * slowIncrement.x + startFromTopRight * (columns - 1),
		j = 0 + applyInitialSlowIncrement * slowIncrement.y;

	for (; i >= 0 && j >= 0 && i < columns && j < rows; i += slowIncrement.x, j += slowIncrement.y) {


		unsigned int x = i, y = j;
		unsigned int counter = 0;
		GAME_CHAR lastSeen = EMPTY;
		for (; x >= 0 && y >= 0 && x < columns && y < rows; x += fastIncrement.x, y += fastIncrement.y) {
			auto val = GetCell(x, y);

			auto winOrLoss = 1 - 2 * CHAR_IS(val, OPPONENT);
			winOrLoss = winOrLoss * CHAR_NOT(CHAR_IS(val, EMPTY));

			counter = (counter * CHAR_IS(lastSeen, val)) + 1; // update counter or reset to 1

			score += (winOrLoss * 2) << counter; // increment/decrement score by 2 << 1 up to 2 << lineLength
			score += ((counter == lineLength) * winOrLoss) << 24; // handle win/loss (counter == lineLength) by adding or subtracting a large number (1 << 24)

			lastSeen = val;
		}
	}
}

void Board::Print() {
	for (unsigned int j = 0; j < GetRows(); j++) {
		for (unsigned int i = 0; i < GetColumns(); i++) {
			cout << GetCell(i, j) << '-';
		}
		cout << endl;
	}
}

unsigned int Board::GetColumns() { return xy >> 16; } // returns x
unsigned int Board::GetRows() { return xy & 0xFFFF; } // returns y

CUDA_CALLABLE_MEMBER Board::~Board()
{
	nDeleted++;
	delete[] data;
	data = nullptr;
}
