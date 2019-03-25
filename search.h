#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "consts.h"

enum Status { Waiting, Playing, Learning };

struct Search
{
    bool infinite = false;
    int think_time = 60000;
    Status status = Waiting;
    int color = WHITE;
    int movecnt = 0;
    int threefold[MAX_PLY];

    Search()
    {}
};

extern Search * S;

// Prototypes //////////////////////

extern void init_search();
extern U64  perft_root(int depth);
extern U64  perft(int depth);
extern void bench();

extern void think();
extern int  pvs(int alpha, int beta, int depth);
extern int  qs(int alpha, int beta);

extern bool repetition();
extern void update_killers(Move move, int depth, int ply);
extern void update_pv(Move move);
extern void print_pv(bool final = false);
extern void out_pv();

extern int  contempt();
extern bool is_endgame();


#endif // SEARCH_H
