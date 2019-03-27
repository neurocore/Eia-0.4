#ifndef BOARD_H
#define BOARD_H

#include <string>
#include "types.h"
#include "consts.h"
#include "moves.h"

using namespace std;

struct State // POD-type
{
    int ep, fifty, cap;
    int castling, pst;
    U64 hash, matkey;
    Move curr;
};

struct Board
{
    U64 piece[PIECE_N];
    int count[PIECE_N];
    U64 occ[COLOR_N];
    int sq[SQUARE_N];
    int wtm = 1;
    State undo[MAX_PLY];
    State * state = undo;

    void clear();
    void print();
    void reset();
    void from_fen(const char * fen);
    string make_fen();

    U64  calc_hash();
    bool try_parse(string str, Move & move);
    bool is_allowed(Move move);

    bool is_attacked(int ksq, U64 occupied);
    bool is_pinned(int ksq, U64 occupied, U64 captured, U64 & att);
    int  cnt_attacks(int ksq, U64 occupied, U64 captured, U64 & att);
    U64  get_attack(int piece, int sq);
    U64  get_attacks_xray(int sq, U64 occupied, int col = 2);
    bool in_check();
    int  see(Move move);
    bool insufficient_material();

    MoveVal * generate(MoveVal * moves);

    bool make(int move, bool self = false);
    void unmake(int move);

    template<bool full = true>
    void place(int square, int p);

    template<bool full = true>
    void remove(int square);
};

template<bool full>
inline void Board::place(int square, int p)
{
    ASSERT(p != NOP);

    piece[p]    ^= (BIT << square);
	occ[COL(p)] ^= (BIT << square);
	sq[square] = p;

    if (full)
    {
        state->hash ^= hashKey[p][square];
        //if (IS_PAWN(p)) state->phash ^= hashKey[p][square];
    }
}

template<bool full>
inline void Board::remove(int square)
{
    int p = sq[square];
    ASSERT(p != NOP);

    /*vector<string> v;
    if (p == NOP)
    {
        for (Node * M = N; M >= S->node; M--)
        {
            stringstream ss;
            ss << M->curr;
            v.push_back(ss.str());
        }
    }*/

    piece[p]    ^= (BIT << square);
	occ[COL(p)] ^= (BIT << square);
	sq[square] = NOP;

    if (full)
    {
        state->hash ^= hashKey[p][square];
        //if (IS_PAWN(p)) state->phash ^= hashKey[p][square];
    }
}

extern Board * B;


#endif // BOARD_H