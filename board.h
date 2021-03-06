#ifndef BOARD_H
#define BOARD_H

#include <string>
#include "types.h"
#include "consts.h"
#include "movelist.h"
#include "moves.h"
#include "eval.h"
#include "magics.h"

using namespace std;

struct State // POD-type
{
    int ep, fifty, cap;
    int castling;
    Val pst;
    U64 hash, mkey;
    Move curr, best;
    Move killer[2];

    int checks;
    U64 pinned, checkers;
    U64 pins[64];
    MoveList ml;
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
    int history[PIECE_N][SQUARE_N];

    void clear();
    void print();
    void reset();
    void from_fen(const char * fen);
    string make_fen();

    U64  calc_hash();
    bool try_parse(string str, Move & move);
    bool is_allowed(Move move);

    bool is_attacked(int ksq, U64 occupied, int opposite = 0);
    bool is_pinned(int ksq, U64 occupied, U64 captured, U64 & att);
    int  cnt_attacks(int ksq, U64 occupied, U64 captured, U64 & att);
    U64  get_attackers(int sq, int col);
    U64  get_attack(int piece, int sq);
    U64  attackers_of(int sq, U64 o);
    U64  get_attacks_xray(int sq, U64 occupied, int col = 2);
    bool in_check();
    U64  get_least_valuable_piece(U64 attadef, int col, int & p);
    int  see(Move move);
    bool insufficient_material();
    void update_killers(Move move, int depth);

    void update_mat_pst();
    void update_tactics();

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
        state->pst += E->pst[p][square];
        state->mkey += matkey[p];

        state->hash ^= hash_key[p][square];
        //if (IS_PAWN(p)) state->phash ^= hashKey[p][square];
    }
}

template<bool full>
inline void Board::remove(int square)
{
    int p = sq[square];
    ASSERT(p != NOP);

    piece[p]    ^= (BIT << square);
	occ[COL(p)] ^= (BIT << square);
	sq[square] = NOP;

    if (full)
    {
        state->pst -= E->pst[p][square];
        state->mkey -= matkey[p];

        state->hash ^= hash_key[p][square];
        //if (IS_PAWN(p)) state->phash ^= hashKey[p][square];
    }
}

extern Board * B;

extern U64 ratt(int sq, U64 o);
extern U64 batt(int sq, U64 o);
extern U64 qatt(int sq, U64 o);


#endif // BOARD_H
