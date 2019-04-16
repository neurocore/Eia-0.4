#include <algorithm>
#include "types.h"
#include "piece.h"
#include "moves.h"
#include "board.h"
#include "util.h"

using namespace std;

int uncastle[64];

bool operator < (const MoveVal & a, const MoveVal & b)
{
    return a.val > b.val;
}

void init_moves()
{
	for (int i = 0; i < 64; i++)
		uncastle[i] = C_ALL;

	uncastle[A1] ^= C_WQ;
	uncastle[E1] ^= C_WQ | C_WK;
	uncastle[H1] ^= C_WK;

	uncastle[A8] ^= C_BQ;
	uncastle[E8] ^= C_BQ | C_BK;
	uncastle[H8] ^= C_BK;
}

#define MOVE_OUT(from, to, a, b) { ss << SQ_OUT(from) << (a) << SQ_OUT(to) << (b); }

string to_string(Move move)
{
	if (move == MOVE_NONE) return string("[NONE]");
    if (move == MOVE_NULL) return string("[NULL]");

    stringstream ss;
    int to = TO(move);
    int from = FROM(move);
    int piece = B->sq[from];
	ss << " " << "pPnNbBrRqQkK."[piece];

	switch (FLAGS(move))
	{
		case F_CAP:      MOVE_OUT(from, to, "x", " "); break;
		case F_EP:       MOVE_OUT(from, to, "x", "e"); break;
		case F_NPROM:    MOVE_OUT(from, to, "", "n "); break;
		case F_BPROM:    MOVE_OUT(from, to, "", "b "); break;
		case F_RPROM:    MOVE_OUT(from, to, "", "r "); break;
		case F_QPROM:    MOVE_OUT(from, to, "", "q "); break;
		case F_NCAPPROM: MOVE_OUT(from, to, "x", "n"); break;
		case F_BCAPPROM: MOVE_OUT(from, to, "x", "b"); break;
		case F_RCAPPROM: MOVE_OUT(from, to, "x", "r"); break;
		case F_QCAPPROM: MOVE_OUT(from, to, "x", "q"); break;
        default:         MOVE_OUT(from, to, "", "  "); break;
	}
	return ss.str();
}

ostream & operator << (ostream & os, const Move & move)
{
    os << to_string(move);
    return os;
}

ofstream & operator << (ofstream & os, const Move & move)
{
    os << to_string(move);
    return os;
}

void order(MoveVal * start, MoveVal * end, Move hash_move)
{
    for (MoveVal * mv = start; mv != end; mv++)
    {
        if (mv->move == hash_move) { mv->val = 1000000; continue; }

        mv->val = 0;
        int flags = FLAGS(mv->move);
        int from = FROM(mv->move);
        int to = TO(mv->move);

        if (IS_PROM(flags)) mv->val += 10000 + N_PROM(flags);

        int a = B->sq[from]; // attacker
        int v = B->sq[to];  // victim

        if (IS_CAP(flags)) mv->val += ABS(E->mat[v]) + 6 - ABS(E->mat[a]) / 100;
        if (IS_CASTLE(flags)) mv->val += 10;
    }

    sort(start, end);

    /*if (moves->val != 0)
    {
        for (MoveVal * mv = moves; mv != end; mv++)
        {
            CON(mv->move << " - " << mv->val << "\n");
        }
        MoveVal * mvv = moves + 1;
        int dfah = 0;
    }*/
}

#undef MOVE_OUT