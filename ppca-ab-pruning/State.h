#pragma once

#include "board.h"

class State
{
public:
    State(GAME_CHAR player);
    State(GAME_CHAR player, const Board& board);
    ~State();

    inline void SetScore(const long long int& score) { this->score = score; }
    inline long long int GetScore() const { return score; }

    inline Board& GetBoard() { return board; }

private:

    Board board;
    long long int score;
};

