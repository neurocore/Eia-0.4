#ifndef SEARCH_H
#define SEARCH_H

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "types.h"
#include "consts.h"
#include "timer.h"

enum Status { Waiting, Playing, Learning };

struct Search
{
    Status status = Waiting;
    bool infinite = false;
    int think_time = 60000;
    int input_time = 1000;
    U64 nodes = EMPTY;
    int search_depth;
    Timer timer;

    int color = WHITE;
    int movecnt = 0;
    int threefold[MAX_PLY];
    Move best;

    U64 hash_read = EMPTY;
    U64 hash_write = EMPTY;

    ofstream flog;

    Search()
    {
#ifdef LOGGING
        time_t now = time(NULL);
        tm Tm[1];
        localtime_s(Tm, &now);
        stringstream ss;
        ss << "log_" << setfill('0')
        << setw(4) << (Tm->tm_year + 1900)
        << setw(2) << (Tm->tm_mon + 1)
        << setw(2) << Tm->tm_mday
        << "_"
        << setw(2) << Tm->tm_hour
        << setw(2) << Tm->tm_min
        << ".txt" << setfill(' ');
 
        flog.open(ss.str());
#endif
    }
};

extern Search * S;

// Prototypes //////////////////////

extern void init_search();
extern U64  perft_root(int depth);
extern U64  perft(int depth);
extern void bench();

extern void think();
extern int  pvs(int alpha, int beta, int depth);
extern int  qs(int alpha, int beta, int qply = 0);

extern bool repetition();
extern void update_killers(Move move, int depth, int ply);
extern void update_pv(Move move);
extern void print_pv(bool final = false);
extern void out_pv();

extern int  contempt();
extern bool is_endgame();


#endif // SEARCH_H
