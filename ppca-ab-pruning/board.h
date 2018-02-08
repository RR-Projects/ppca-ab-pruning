#pragma once

#include <iostream>
#include <memory>

#include "ppca_helpers.h"
#include "Point.h"

using namespace std;

class Board
{
public:
    static unsigned long long nCreated, nCopied, nDeleted;

    Board();
    Board(int* data);
    Board(const int& x, const int& y, const unsigned char& lineLength);

    Board(const Board& b);
    Board(Board&& b);

    Board& operator=(const Board& b);

    ~Board();

    inline unsigned int GetFilledCells() const { return filledCells; }
    inline unsigned int GetMaxDepth() const { return columns * rows - filledCells; }

    inline void SetTreePosition(const unsigned int& pos) { treePosition = pos; }
    inline unsigned int GetTreePosition() const { return treePosition; }

    inline bool HasNextMove() const { return GetUpper16Bits(nextMoveIterator) < columns && GetLower16Bits(nextMoveIterator) < rows && (!IsTerminal()); }
    void GetNextMove(Board& move, GAME_CHAR player);
    inline void ResetMoveIterator() { nextMoveIterator = 0; }

    int GenerateMoves(Board** results, GAME_CHAR player);

    inline unsigned int GetColumns() const { return columns; }
    inline unsigned int GetRows() const { return rows; }

    inline unsigned int GetCell(const int& x, const int& y) const {
        return GetCellInternal(GetIndex(x, y), GetOffset(x, y));
    }

    void SetCell(int x, int y, unsigned int val);
    void Print() const;

    void CalculateScore();
    inline long long GetScore() const { return partialScore; }
    inline void SetScore(const long long& _score) { partialScore = _score; }

    inline bool IsTerminal() const { return isTerminal || (columns * rows == filledCells); }

    inline GAME_CHAR GetWinner() const { return winner; }

    inline unsigned int GetDataStructureSize() const {
        return ((this->columns * this->rows << 1) + 31) / (sizeof(int) << 3); // (x * y * 2 + 31) / (8 * sizeof(int)); 
    }

    inline unsigned int GetSizeForBuffer() const { return GetDataStructureSize() + 2; }

    unsigned int* CopyToBuffer(unsigned int* buffer) const;
private:
    unsigned int* data;
    unsigned int filledCells, treePosition, nextMoveIterator;
    unsigned char columns, rows, lineLength, isTerminal;
    long long partialScore;
    GAME_CHAR winner;
    // min size of class: 16 bytes (4 * (1 int of data + 3 int properties))

    void CalculateScoreOnDirection(Point slowIncrement, Point fastIncrement, bool startFromTopRight = false, bool applyInitialSlowIncrement = false);

    inline unsigned int GetCellInternal(const int& index, const char& offset) const {
        return (data[index] >> offset) & 0x3;
    }

    inline unsigned int GetIndex(const int& x, const int& y) const {
        return this->pos(x, y) / 32;
    }

    inline unsigned int GetOffset(const int& x, const int& y) const {
        return this->pos(x, y) % 32;
    }

    inline unsigned int pos(const int& x, const int& y) const {
        return (this->columns * y + x) << 1;
    }

    inline unsigned int GetUpper16Bits(const unsigned int& val) const { return val >> 16; }
    inline unsigned int GetLower16Bits(const unsigned int& val) const { return val & 0xFFFF; }
};
