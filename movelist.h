#ifndef MOVELIST_H
#define MOVELIST_H

#include "consts.h"
#include "moves.h"

enum Stage
{
    S_HASH,
    S_GEN_CAPS,
    S_WIN_CAPS,
    S_EQ_CAPS,
    S_KILLER1,
    S_KILLER2,
    S_GEN_QUIET,
    S_QUIET,
    S_BAD_CAPS
};

struct MoveList
{
    Move hashmove;
    MoveVal moves[256];
    MoveVal * first = moves;
    MoveVal * last = moves;
    Stage stage = S_HASH;

    MoveList(Move hashmove = MOVE_NONE)
        : hashmove(hashmove)
    {}

    void init(int hash_move = MOVE_NONE);

    Move get_best_move(int lower_bound = -INT_MAX);

    void add(Move move);
    void print();
    Move get_next_move();

    void remove(MoveVal * mv);
    void remove_hashmove();
    void remove_hashmove_and_killers();

    void set_values();
    void generate_all();
    void generate_attacks();
    void generate_quiets();
    void generate_evasions();
    void sort();
};


#endif // MOVELIST_H