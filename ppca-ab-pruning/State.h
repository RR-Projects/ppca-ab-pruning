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

    inline void SetAlpha(const long long int& alpha) { this->alpha = alpha; }
    inline long long int GetAlpha() { return alpha; }

    inline void SetBeta(const long long int& beta) { this->beta = beta; }
    inline long long int GetBeta() { return beta; }

    inline Board& GetBoard() { return board; }

private:

    Board board;
    long long int score, alpha, beta;
};

