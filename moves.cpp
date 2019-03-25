#include <ostream>
#include "types.h"
#include "piece.h"
#include "moves.h"
#include "board.h"
#include "util.h"

using namespace std;

int uncastle[64];

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

#define MOVE_OUT(from, to, a, b) { CON(SQ_OUT(from) << (a) << SQ_OUT(to) << (b)); }

ostream & operator << (ostream & os, const Move & move)
{
	if (!IS_VALID(move))
	{
		os << "[Invalid move]";
		return os;
	}

    int to = TO(move);
    int from = FROM(move);
    int piece = B->sq[from];
	os << " " << "pPnNbBrRqQkK."[piece];

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
	}

	return os;
}

#undef MOVE_OUT