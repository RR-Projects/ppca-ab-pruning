#pragma once

#include "board.h"

class State
{
public:
    State(GAME_CHAR player);
    State(GAME_CHAR player, const Board& board);
    ~State();

    inline void SetScore(const long long& score) { this->score = score; }
    inline long long GetScore() const { return score; }

    inline void SetAlpha(const long long& alpha) { this->alpha = alpha; }
    inline long long GetAlpha() { return alpha; }

    inline void SetBeta(const long long& beta) { this->beta = beta; }
    inline long long GetBeta() { return beta; }

    inline Board& GetBoard() { return board; }

private:

    Board board;
    long long score, alpha, beta;
};

