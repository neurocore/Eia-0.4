#include "search.h"
#include "board.h"
#include "timer.h"

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
    MoveVal * end = B->generate(moves);

	for (MoveVal * mv = moves; mv != end; mv++)
	{
		B->state->curr = mv->move;
		CON(mv->move << "(" << int(mv->move) << ") - ");
        if (!B->make(mv->move)) continue;
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
    MoveVal * end = B->generate(moves);

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
}

void think()
{
}
