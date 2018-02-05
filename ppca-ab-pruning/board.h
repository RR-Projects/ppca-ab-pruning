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

    CUDA_CALLABLE_MEMBER Board();
    CUDA_CALLABLE_MEMBER Board(const int& x, const int& y, const unsigned char& lineLength);

    CUDA_CALLABLE_MEMBER Board(const Board& b);
    CUDA_CALLABLE_MEMBER Board(Board&& b);

    Board& operator=(const Board& b);

    CUDA_CALLABLE_MEMBER ~Board();

    void SetTreePosition(const unsigned int& pos) { treePosition = pos; }
    unsigned int GetTreePosition() const { return treePosition; }

    void GetNextMove(Board& move, GAME_CHAR player);
    bool HasNextMove() const { return GetUpper16Bits(nextMoveIterator) < GetUpper16Bits(xy) && GetLower16Bits(nextMoveIterator) < GetLower16Bits(xy) && (!IsTerminal()); }
    void ResetMoveIterator() { nextMoveIterator = 0; }

    int GenerateMoves(Board** results, GAME_CHAR player);

    unsigned int inline GetColumns() const { return GetUpper16Bits(xy); }
    unsigned int inline GetRows() const { return GetLower16Bits(xy); }

    unsigned int inline GetCell(const int& x, const int& y) const {
        return GetCellInternal(GetIndex(x, y), GetOffset(x, y));
    }

    void SetCell(int x, int y, unsigned int val);
    void Print() const;

    void CalculateScore();
    long long int inline GetScore() const { return score; }
    void inline SetScore(const long long int& _score) { score = _score; }

    bool inline IsTerminal() const { return isTerminal || (GetColumns() * GetRows() == filledCells); }
private:
    unsigned int* data;
    unsigned int xy, filledCells, treePosition, nextMoveIterator;
    unsigned char lineLength, isTerminal;
    long long int score;
    // min size of class: 16 bytes (4 * (1 int of data + 3 int properties))

    int alpha, beta;

    void CalculateScoreOnDirection(Point slowIncrement, Point fastIncrement, bool startFromTopRight = false, bool applyInitialSlowIncrement = false);

    unsigned int inline GetCellInternal(const int& index, const char& offset) const {
        return (data[index] >> offset) & 0x3;
    }

    unsigned int inline GetIndex(const int& x, const int& y) const {
        return this->pos(x, y) / 32;
    }

    unsigned int inline GetOffset(const int& x, const int& y) const {
        return this->pos(x, y) % 32;
    }

    unsigned int inline pos(const int& x, const int& y) const {
        return (GetColumns() * y + x) << 1;
    }

    unsigned int inline GetDataStructureSize() const {
        return ((GetColumns() * GetRows() << 1) + 31) / (sizeof(int) << 3); // (x * y * 2 + 31) / (8 * sizeof(int)); 
    }

    unsigned int inline GetUpper16Bits(const unsigned int& val) const { return val >> 16; }
    unsigned int inline GetLower16Bits(const unsigned int& val) const { return val & 0xFFFF; }
};
