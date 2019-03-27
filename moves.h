#ifndef MOVES_H
#define MOVES_H

struct MoveVal
{
    Move move;
    int val;
};

extern int uncastle[64];

extern void init_moves();
extern ostream & operator << (ostream & os, const Move & move);


#endif // MOVES_H
