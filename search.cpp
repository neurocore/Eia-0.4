#include "movelist.h"
#include "options.h"
#include "search.h"
#include "board.h"
#include "timer.h"
#include "consts.h"
#include "eval.h"
#include "hash.h"
#include "uci.h"

string code(Move move)
{
    ostringstream oss;
    oss << "[" << int(move) << "]";
    return oss.str();
}

void init_search()
{
}

U64 perft_root(int depth)
{
    U64 cnt = EMPTY;
	CON("-- Perft " << depth << " --\n\n");
	B->print();

	Timer timer;
	timer.set();

    B->state->ml.init();
    // position fen r1bq1rk1/ppp2ppp/2n1pn2/3p2Q1/3P4/P1P2N2/2P1PPPP/R1B1KB1R w KQ - 0 8

    while (Move move = B->state->ml.get_next_move())
	{
		B->state->curr = move;
        if (!B->make(move)) continue;
		CON(move << setw(7) << code(move) << " - ");
		U64 prft = perft(depth - 1);
		cnt += prft;
		CON(prft << "\n");
		B->unmake(move);
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

    B->state->ml.init();

	U64 cnt = EMPTY;
    while (Move move = B->state->ml.get_next_move())
	{
        //CON(move << "\n");
        if (!B->make(move)) continue;
        if (depth > 1) cnt += perft(depth - 1);
        else cnt++;
        B->unmake(move);
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
    B->state->best = MOVE_NONE;

    B->undo[0].killer[0] = B->undo[2].killer[0];
    B->undo[0].killer[1] = B->undo[2].killer[1];

    for (int p = 0; p < PIECE_N; p++)
        for (int s = 0; s < 64; s++)
            B->history[p][s] = 0;

    for (int i = 0; i < MAX_PLY; i++)
    {
        State & s = B->undo[i];
        s.curr = s.best = MOVE_NONE;
    }

    for (int i = 0; i < MAX_PLY; i++)
        B->undo[i].killer[0] = B->undo[i].killer[1] = MOVE_NONE;

    int alpha = -INF, beta = INF;

	for (int iter = 1; iter <= MAX_PLY; iter++)
	{
        int depth = iter;
		S->search_depth = depth;
		int val = pvs(alpha, beta, depth);
		if (!S->status) break;

        string comment = "";
        if (val > alpha && val < beta) // Normal
        {
            alpha = val - ROOT_WINDOW / 2;
            beta  = val + ROOT_WINDOW / 2;
        }
        else                           // Unexpectable
        {
            comment = (val <= alpha) ? "?" : "!";
            alpha = -INF;
            beta  =  INF;
            iter--;
        }

        if (IS_VALID(B->state->best))
        {
		    int ms = S->timer.getms();
            OUT("info depth " << depth << " pv " << MV_OUT(B->state->best) << comment << " ");
            //out_pv();

            if (val > MATE) OUT("score mate " << (INF - val) / 2 + 1 << "\n")
            else if (val < -MATE) OUT("score mate " << -(INF + val) / 2 - 1 << "\n")
            else                  OUT("score cp " << val << "\n");

		    OUT("info time " << ms <<
                " nodes " << S->nodes << 
                " nps " << 1000 * S->nodes / NOTZERO(ms) <<
                " cpuload 1000\n");
        }

        //CON("hash (r/w): " << S->hash_read << " / " << S->hash_write << "\n");

		if (IS_VALID(B->state->best)) S->best = B->state->best;

		if (val >  MATE) if (INF - val <= depth) break;
		if (val < -MATE) if (INF + val <= depth) break;

        FLUSH;
	}
	OUT("bestmove " << MV_OUT(S->best) << "\n");
    FLUSH;
}

int pvs(int alpha, int beta, int depth)
{
    if (depth <= 0) return qs(alpha, beta);

    bool search_pv = true;
    B->state->best = Move();
    int hash_type = Hash_Alpha; 
    int val = -INF;
    S->nodes++;

    if (!S->status || time_to_answer()) return 0;
    if (B->state - B->undo >= MAX_PLY) return 0;

    bool in_check = B->state->checks;
	bool in_pv = (beta - alpha) > 1;
	bool late_endgame = MATERIAL(B->wtm)->phase < 5;

    // 1.1. Mate pruning /////////////////////////////////////

#ifdef SEARCH_MATE_PRUNING
	if (PLY > 0)
	{
		alpha = MAX(-INF + PLY, alpha);
		beta = MIN(INF - (PLY + 1), beta);
		if (alpha >= beta) return alpha;
	}
#endif

    // 1.2. Hash probe ///////////////////////////////////////

	HashEntry * he = 0;
    Move hash_move = MOVE_NONE;

#ifdef SEARCH_HASHING
	he = H->get(B->state->hash);
    if (he)
    {
        if (IS_VALID(he->move)
        &&  B->is_allowed(Move(he->move)))
            hash_move = Move(he->move);

        if (PLY > 0) // Not in root
        {
            if (he->depth >= depth)
            {
                int val = he->val;
                if      (val >  MATE && val <=  INF) val -= PLY;
                else if (val < -MATE && val >= -INF) val += PLY;

                // Exact score
                if (he->type == Hash_Exact) return val;
                else if (he->type == Hash_Alpha && val <= alpha) return alpha;
                else if (he->type == Hash_Beta  && val >= beta) return beta;
            }
        }
    }
#endif

    // 1.3. Futility pruning //////////////////////////////////

#ifdef SEARCH_FUTILITY
	const int Futility_Margin = 150;
	if (depth <= 1 && !in_check && !in_pv && !late_endgame)
	{
        Material * m[2] = { MAT_BLACK, MAT_WHITE };
		int mat = m[B->wtm]->val - m[B->wtm ^ 1]->val;
		if (mat > beta + Futility_Margin)
			return beta;
	}
#endif
    
    B->state->ml.init(hash_move);

    int legal = 0;
    while (Move move = B->state->ml.get_next_move())
    {
        if (!B->make(move)) continue;

#ifdef DEBUG_MOVES
        for (int i = 0; i < PLY; i++) CON(" ");
        CON(move << "\n");
#endif
        legal++;
        int new_depth = depth - 1;

        bool gives_check = B->state->checks;
        bool reduced = false;

        // 2.0. Late Move Reductions //////////////////////////////////

#ifdef SEARCH_LMR
		if (!in_pv && depth >= 2
        &&  !in_check && !gives_check
        &&  !IS_CAP_OR_PROM(FLAGS(move))
        &&  legal >= 4 && !late_endgame)
		{
			reduced = true;
			new_depth--;
		}
#endif

        if (search_pv)
            val = -pvs(-beta, -alpha, new_depth);
        else
        {
            val = -pvs(-alpha - 1, -alpha, new_depth);
            if (val > alpha && val < beta)
                val = -pvs(-beta, -alpha, new_depth);
        }

        if (reduced && val >= beta)
            val = -pvs(-beta, -alpha, new_depth + 1);

        B->unmake(move);

        if (val > alpha)
        {
            alpha = val;
            hash_type = Hash_Exact;
            B->state->best = move;
            search_pv = false;

            if (val >= beta)
            {
                if (!IS_CAP_OR_PROM(move) && !in_check)
                    B->update_killers(move, depth);

                alpha = beta;
                hash_type = Hash_Beta;
                break;
            }
        }
    }

    if (!legal)
	{
		return in_check > 0 ? -INF + B->state - B->undo : 0; // contempt();
	}

#ifdef SEARCH_HASHING
    H->set(B->state->hash, B->state->best, depth, alpha, hash_type);
#endif

    return alpha;
}

int qs(int alpha, int beta, int qply)
{
    if (!S->status || time_to_answer()) return 0;
    if (B->state - B->undo >= MAX_PLY) return 0;

    MoveList & ml = B->state->ml;
    ml.init();
    S->nodes++;

    if (!B->state->checks)
    {
        int standpat = eval();
        if (standpat > alpha)
        {
            if (standpat >= beta) return standpat;
            alpha = standpat;
        }

        ml.generate_attacks();
        ml.set_values();
        //if (qply < 2) ml.generate_simple_checks();
    }
    else
    {
        ml.generate_all();
        ml.set_values();
    }

    int legal = 0;
    while (Move move = ml.get_best_move())
    {
        if (!B->make(move)) continue;
        legal++;

        int val = -qs(-beta, -alpha, qply + 1);

        B->unmake(move);

        if (val > alpha)
        {
            if (val >= beta) return beta;
            alpha = val;
        }
    }

    if (B->state->checks && !legal)
		alpha = -INF + PLY;

    return alpha;
}
