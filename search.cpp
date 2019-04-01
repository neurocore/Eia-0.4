#include "search.h"
#include "board.h"
#include "timer.h"
#include "consts.h"
#include "eval.h"
#include "uci.h"

void init_search()
{
}

U64 perft_root(int depth)
{
    U64 cnt = EMPTY;
	CON( "-- Perft " << depth << " --\n\n" );
	B->print();

	Timer timer;
	timer.set();

    MoveVal moves[256];
    MoveVal * end = B->generate_all(moves);

	for (MoveVal * mv = moves; mv != end; mv++)
	{
		B->state->curr = mv->move;
        if (!B->make(mv->move)) continue;
		CON(mv->move << " - ");
		U64 prft = perft(depth - 1);
		cnt += prft;
		CON(prft << "\n");
		B->unmake(mv->move);
	}

	double time = timer.get();
	double speed = (cnt / NOTZERO(time)) / 1000;

	CON("\n");
	CON("Total: " << cnt << " nodes\n");
	CON("Time: " << time << " sec\n");
	CON("Speed: " << speed << " knps\n\n\n");

	return cnt;
}

U64 perft(int depth)
{
    if (depth <= 0) return 1;
    //B->state->hash_move = Move();

	MoveVal moves[256];
    MoveVal * end = B->generate_all(moves);

	U64 cnt = EMPTY;
	for (MoveVal * mv = moves; mv != end; mv++)
	{
        if (!B->make(mv->move)) continue;
        if (depth > 1) cnt += perft(depth - 1);
        else cnt++;
        B->unmake(mv->move);
	}
	return cnt;
}

void bench()
{
    for (int i = 1; i <= 6; i++)
        perft_root(i);
}

void think()
{
    S->nodes = 0;
	S->status = Status::Playing;
	S->timer.set();

	for (int i = 1; i <= MAX_PLY; i++)
	{
        //engine->board->print();

		S->search_depth = i;
		int val = pvs(-INF, INF, i);
		if (!S->status) break;

		int ms = S->timer.getms();

		OUT("info depth " << i << " pv " << MV_OUT(B->state->best) << " ");
		//out_pv();

		if      (val >  MATE) OUT("score mate " <<  (INF - val) / 2 + 1 << "\n")
		else if (val < -MATE) OUT("score mate " << -(INF + val) / 2 - 1 << "\n")
		else                  OUT("score cp "   << val << "\n");

		OUT("info time " << ms <<
            " nodes " << S->nodes << 
            " nps " << 1000 * S->nodes / NOTZERO(ms) <<
            " cpuload 1000\n");

        //OUT("info string pawn hash " << engine->phashRead <<
        //    " / " << engine->phashWrite << "\n");

		S->best = B->state->best;

		if (val >  MATE) if (INF - val <= i) break;
		if (val < -MATE) if (INF + val <= i) break;

        FLUSH;
	}
	OUT("bestmove " << MV_OUT(S->best) << "\n");
    FLUSH;
}

int pvs(int alpha, int beta, int depth)
{
    if (depth <= 0) return qs(alpha, beta);
    bool search_pv = true;
    int val = -INF;
    B->state->best = Move();
    S->nodes++;

    if (!S->status || time_to_answer()) return 0;

    MoveVal moves[256];
    MoveVal * end = B->generate(moves);

    int legal = 0;
    for (MoveVal * mv = moves; mv != end; mv++)
    {
        if (!B->make(mv->move)) continue;
        legal++;

        if (search_pv) val = -pvs(-beta, -alpha, depth - 1);
        else
        {
            val = -pvs(-alpha - 1, -alpha, depth - 1);
            if (val > alpha) val = -pvs(-beta, -alpha, depth - 1);
        }
        B->unmake(mv->move);

        if (val >= beta) return beta;
        if (val > alpha)
        {
            alpha = val;
            B->state->best = mv->move;
            search_pv = false;
        }
    }

    if (!legal)
	{
		return B->state->checks > 0 ? -INF + B->state - B->undo : 0; // contempt();
	}

    return alpha;
}

int qs(int alpha, int beta)
{
    return eval();
}
