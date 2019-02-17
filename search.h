#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

struct Node {};
struct Eval
{
    int pst[PIECE_N][SQUARE_N];
};

struct Search
{
    Eval * eval;
};




#endif // SEARCH_H