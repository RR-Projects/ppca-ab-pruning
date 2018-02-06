#pragma once

//#ifdef __CUDACC__
//#define CUDA_CALLABLE_MEMBER __host__ __device__
//#else
//#define CUDA_CALLABLE_MEMBER
//#endif 

#define GAME_CHAR unsigned int

#define EMPTY (GAME_CHAR)0x0
#define PLAYER (GAME_CHAR)0x1
#define OPPONENT (GAME_CHAR)0x2
#define CELL_BITMAP (GAME_CHAR)0x3

#define SWITCH_PLAYER(p) ~p & CELL_BITMAP
#define CHAR_IS(val, comparer) (!(val - comparer))
//((val & comparer & PLAYER) | ((val & comparer & OPPONENT) >> 1) | ((val | comparer | 0x4) & EMPTY) ))
#define CHAR_NOT(val) (!val)