#include "board.h"

unsigned long long Board::nCreated = 0ULL;
unsigned long long Board::nCopied = 0ULL;
unsigned long long Board::nDeleted = 0ULL;

CUDA_CALLABLE_MEMBER Board::Board() : data(nullptr), xy(0), score(0), filledCells(0) { }
CUDA_CALLABLE_MEMBER Board::Board(int x, int y) : filledCells(0), data(nullptr)
{
	nCreated++;
	xy = (x << 16) | (y & 0xFFFF);

	score = x * y;
	auto size = GetSize();
	data = new unsigned int[size];
	memset(data, 0, size * sizeof(int));
}

CUDA_CALLABLE_MEMBER Board::Board(const Board& b) {
	operator=(b);
}

CUDA_CALLABLE_MEMBER Board::Board(Board&& b) : xy(b.xy), filledCells(b.filledCells), score(b.score) {
	data = b.data;
}

Board& Board::operator=(const Board & b)
{
	nCopied++;

	xy = b.xy;
	filledCells = b.filledCells;
	score = b.score;

	auto size = GetSize();
	data = new unsigned int[size];
	memcpy(data, b.data, size * sizeof(int));

	return *this;
}

void Board::GenerateMoves(thrust::host_vector<Board>& result, GAME_CHAR player) {
	result.clear();
	result.resize(GetColumns() * GetRows() - filledCells);

	int idx = 0;

	for (unsigned int j = 0; j < GetRows(); j++) {
		for (unsigned int i = 0; i < GetColumns(); i++) {

			if (GetCell(i, j) == 0) {
				result[idx] = *this;
				result[idx].SetCell(i, j, player);
				idx++;
			}
		}
	}
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

	UpdateScore(x, y);
}

void Board::UpdateScore(int x, int y) {
	score--; // A cell has been occupied so less chances of winning -1

	auto val = GetCell(x, y);

	score += ((val & PLAYER) << 1) - ((val & OPPONENT) >> 1); // if the current cell is PLAYER +2 else if OPPONENT -1


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
