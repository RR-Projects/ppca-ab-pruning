#include "State.h"


State::State(GAME_CHAR player) : State(player, board) { }

State::State(GAME_CHAR player, const Board& board)
{
    alpha = INT64_MIN;
    beta = INT64_MAX;

    score = CHAR_IS(player, PLAYER) * alpha
        + CHAR_IS(player, OPPONENT) * beta;

    this->board = board;
}

State::~State()
{
}
