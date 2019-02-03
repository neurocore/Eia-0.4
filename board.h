#ifndef BOARD_H
#define BOARD_H

#include <string>
#include "types.h"

using namespace std;

struct Search;
struct Board
{
    U64 piece[12];
    U64 occ[2];
    int sq[64];
    int wtm;

    //Board();
    void clear();
    void print();
    void reset(Search * S);
    void fromFen(const char * fen, Search * S);
    string makeFen(Search * S);

    bool isAttacked(int ksq, U64 occupied, U64 captured);
    bool isPinned(int ksq, U64 occupied, U64 captured, U64 & att);
    int  cntAttacks(int ksq, U64 occupied, U64 captured, U64 & att);
    U64  getAttacksXray(int sq, U64 occupied, int col = 2);
    bool inCheck();
    int  see(Move move);
    bool insufficientMaterial();

    /*template<bool full = true>
    void place(int sq, int p, Node * N = 0, Eval * E = 0);

    template<bool full = true>
    void remove(int sq, Node * N = 0, Eval * E = 0);*/

    U64  getAttack(int p, int sq);
};




#endif // BOARD_H