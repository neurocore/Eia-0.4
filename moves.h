#ifndef MOVES_H
#define MOVES_H

#include <sstream>
#include <fstream>
#include <ostream>

using namespace std;

struct MoveVal
{
    Move move;
    int val;

    friend bool operator < (const MoveVal & a, const MoveVal & b);
};

extern int uncastle[64];

extern void init_moves();
extern string to_string(Move move);
extern ostream & operator << (ostream & os, const Move & move);
extern ofstream & operator << (ofstream & os, const Move & move);


#endif // MOVES_H
