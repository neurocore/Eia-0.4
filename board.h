#ifndef BOARD_H
#define BOARD_H

#include <string>
#include "types.h"

using namespace std;

struct Board
{
    struct State // POD-type
	{
        int ep, fifty;
        int castling, pst;
        U64 hash;
	};

    U64 piece[PIECE_N];
    int count[PIECE_N];
    U64 occ[COLOR_N];
    int sq[SQUARE_N];
    int wtm, ply;
    State state;
    State undo[MAX_PLY];

    void clear();
    void print();
    void reset();
    void fromFen(const char * fen);
    string makeFen();

    bool isAttacked(int ksq, U64 occupied, U64 captured);
    bool isPinned(int ksq, U64 occupied, U64 captured, U64 & att);
    int  cntAttacks(int ksq, U64 occupied, U64 captured, U64 & att);
    U64  getAttacksXray(int sq, U64 occupied, int col = 2);
    bool inCheck();
    int  see(Move move);
    bool insufficientMaterial();

    void make(int move);
    void unmake(int move);

    U64  getAttack(int p, int sq);
};




#endif // BOARD_H