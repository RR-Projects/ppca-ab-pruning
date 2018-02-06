#include "board.h"

unsigned long long Board::nCreated = 0ULL;
unsigned long long Board::nCopied = 0ULL;
unsigned long long Board::nDeleted = 0ULL;

CUDA_CALLABLE_MEMBER Board::Board() : data(nullptr), xy(0), score(0), filledCells(0), lineLength(0), isTerminal(false) { }
CUDA_CALLABLE_MEMBER Board::Board(const int& x, const int& y, const unsigned char& lineLength) : filledCells(0), data(nullptr), lineLength(lineLength), treePosition(0), nextMoveIterator(0), isTerminal(false)
{
    nCreated++;
    xy = (x << 16) | (y & 0xFFFF);

    score = x * y;
    auto size = GetDataStructureSize();
    data = new unsigned int[size];
    memset(data, 0, size * sizeof(int));
}

CUDA_CALLABLE_MEMBER Board::Board(const Board& b) {
    operator=(b);
}

CUDA_CALLABLE_MEMBER Board::Board(Board&& b) : xy(b.xy), filledCells(b.filledCells), score(b.score), lineLength(b.lineLength), treePosition(b.treePosition), nextMoveIterator(0), isTerminal(b.isTerminal) {
    data = b.data;
}

Board& Board::operator=(const Board & b)
{
    nCopied++;

    nextMoveIterator = 0;

    isTerminal = b.isTerminal;
    lineLength = b.lineLength;
    xy = b.xy;
    filledCells = b.filledCells;
    treePosition = b.treePosition;
    score = b.score;

    auto size = GetDataStructureSize();
    data = new unsigned int[size];
    memcpy(data, b.data, size * sizeof(int));

    return *this;
}

void Board::GetNextMove(Board& move, GAME_CHAR player) {
    auto columns = GetColumns(), rows = GetRows();
    auto boardSize = columns * rows;

    unsigned int
        i = GetUpper16Bits(nextMoveIterator),
        j = GetLower16Bits(nextMoveIterator);

    bool foundMove = false;

    for (; j < rows; j++) {
        for (; i < columns; i++) {

            if (GetCell(i, j) == 0) {

                // found second move, update iterator and return
                if (foundMove) {
                    nextMoveIterator = (i << 16) | (j & 0xFFFF);
                    return;
                }
                foundMove = true;

                // found first move, set it to the return parameter
                move = *this;
                move.SetCell(i, j, player);
                move.SetTreePosition(boardSize * treePosition + columns * j + i + 1);
            }
        }
        i = 0;
    }

    nextMoveIterator = (columns << 16) | (rows & 0xFFFF);
}

int Board::GenerateMoves(Board** results, GAME_CHAR player) {

    auto columns = GetColumns(), rows = GetRows();
    auto boardSize = columns * rows;

    *results = new Board[boardSize - filledCells];
    auto _results = *results;

    int idx = 0;

    ResetMoveIterator();
    while (HasNextMove()) {
        GetNextMove(_results[idx], player);
        idx++;
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

    CalculateScore();
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

            score += (winOrLoss * 4) << counter; // increment/decrement score by 2 << 1 up to 2 << lineLength
            score += ((counter == lineLength) * winOrLoss) << 24; // handle win/loss (counter == lineLength) by adding or subtracting a large number (1 << 24)
            isTerminal |= (counter == lineLength) * CHAR_NOT(CHAR_IS(val, EMPTY));

            lastSeen = val;
        }
    }
}

void Board::Print() const {
    for (unsigned int j = 0; j < GetRows(); j++) {
        for (unsigned int i = 0; i < GetColumns(); i++) {
			switch (GetCell(i, j))
			{
			case 0:
				cout << '-';
				break;
			case 1:
				cout << 'X';
				break;
			case 2:
				cout << '0';
				break;
			default:
				break;
			}
            
        }
        cout << endl;
    }
}

CUDA_CALLABLE_MEMBER Board::~Board()
{
    nDeleted++;
    delete[] data;
    data = nullptr;
}
