#include "State.h"


State::State(GAME_CHAR player) : State(player, board) { }

State::State(GAME_CHAR player, const Board& board)
{
    score = CHAR_IS(player, PLAYER) * INT64_MIN
        + CHAR_IS(player, OPPONENT) * INT64_MAX;
    this->board = board;
}

State::~State()
{
}
