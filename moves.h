#ifndef MOVES_H
#define MOVES_H

#include <sstream>
#include <fstream>
#include <ostream>
#include "types.h"

using namespace std;

enum Ordering
{
    O_HASH    =   0x40000000,
    O_WIN_CAP =   0x30000000,
    O_EQ_CAP  =   0x20000000,
    O_KILLER1 =   0x10000001,
    O_KILLER2 =   0x10000000,
    O_CASTLE  =   0x00000100,
    O_QUIET   =   0x00000000,
    O_BAD_CAP = - 0x10000000,
};

struct MoveVal
{
    Move move;
    int val;

    MoveVal(Move move = Move(), int val = 0) : move(move), val(val) {}
    friend bool operator == (const MoveVal & a, const MoveVal & b);
    friend bool operator <  (const MoveVal & a, const MoveVal & b);
};

extern int uncastle[64];

extern void init_moves();
extern string to_string(Move move);
extern ostream & operator << (ostream & os, const Move & move);
extern ofstream & operator << (ofstream & os, const Move & move);


#endif // MOVES_H
