#include <algorithm>
#include "movelist.h"
#include "magics.h"
#include "board.h"
#include "piece.h"
#include "util.h"

using namespace std;

void MoveList::init(int hash_move)
{
    first = last = moves;
    stage = S_HASH;
    hashmove = Move(hash_move);
}

Move MoveList::get_best_move(int lower_bound)
{
    if (first == last) return MOVE_NONE;

    MoveVal * best = first;
    for (MoveVal * mv = first; mv != last; mv++)
        if (mv->val > best->val) best = mv;

    if (best->val >= lower_bound)
    {
        Move move = best->move;
        *best = *(--last); // Removing best move completely
        return move;
    }

    swap(best, first); // Putting best move into the pocket
    first++;           //   best <- [.. movelist ..]
    return MOVE_NONE;  // This stage is finished
}

inline void MoveList::add(Move move)
{
    (last++)->move = move;
}

void MoveList::print()
{
    for (MoveVal * mv = first; mv != last; mv++)
        CON("| " << mv->move << " - " << mv->val << "\n");
    CON("\n");
}

Move MoveList::get_next_move()
{
    switch (stage)
    {
        case S_HASH:
            ASSERT(first == moves && last == moves);
            stage = S_GEN_CAPS;
            first = last = moves;
            if (hashmove != MOVE_NONE) return hashmove;

        case S_GEN_CAPS:
            stage = S_WIN_CAPS;
            generate_attacks();
            remove_hashmove();
            set_values();

        case S_WIN_CAPS:
            if (Move move = get_best_move(O_WIN_CAP)) return move;
            stage = S_EQ_CAPS;

        case S_EQ_CAPS:
            if (Move move = get_best_move(O_EQ_CAP)) return move;
            stage = S_GEN_QUIET;

        case S_GEN_QUIET:
            stage = S_QUIET;
            generate_quiets();
            remove_hashmove();
            set_values();

        case S_QUIET:
            if (Move move = get_best_move(O_QUIET)) return move;
            stage = S_BAD_CAPS;
            first = moves; // Activate pocket moves

        case S_BAD_CAPS:
            return get_best_move();
    }
    return MOVE_NONE;
}

void MoveList::remove(MoveVal * mv)
{
    *mv = *(--last);
}

void MoveList::remove_hashmove()
{
    for (MoveVal * mv = first; mv != last; mv++)
    {
        if (mv->move == hashmove)
        {
            *mv = *(--last);
            return;
        }
    }
}

void MoveList::remove_hashmove_and_killers()
{
    for (MoveVal * mv = first; mv != last;)
    {
        if (mv->move == hashmove
        ||  mv->move == B->state->killer[0]
        ||  mv->move == B->state->killer[1])
            *mv = *(--last);
        else mv++;
    }
}

void MoveList::set_values()
{
    const int cost[] = { 100, 100, 300, 300, 350, 350, 500, 500, 900, 900, 10000, 10000 };
    const int prom[] = { cost[WN], cost[WB], cost[WR], cost[WQ] };

    for (MoveVal * mv = first; mv != last; mv++)
    {
        if (mv->move == hashmove) { mv->val = O_HASH; continue; }
        if (mv->move == B->state->killer[0]) { mv->val = O_KILLER1; continue; }
        if (mv->move == B->state->killer[1]) { mv->val = O_KILLER2; continue; }

        int flags = FLAGS(mv->move);
        int from = FROM(mv->move);
        int to = TO(mv->move);

        if (IS_CAP_OR_PROM(flags))
        {
            /**
             *  Actually, val = C + 100 * gaining - losing, where
             *    gaining = victim + promoted
             *    losing = attacker
             *  
             *  As written above, measure of material we gain is decisive
             *    in cap-prom-ordering. Generally, getting most valuable
             *    victim and most valuable promotion piece is a good idea.
             */

            int p = prom[N_PROM(flags)]; // promoted
            int a = cost[B->sq[from]];  // attacker
            int v = cost[B->sq[to]];   // victim

            if     (IS_PROM(flags)) mv->val = O_WIN_CAP + 100 * (p + v) - a;
            else if (IS_CAP(flags)) mv->val = COMPARE(v, a, O_BAD_CAP, O_EQ_CAP, O_WIN_CAP) + 100 * v - a;
        }
        else
        {
            if (IS_CASTLE(flags)) mv->val = O_CASTLE;
            else if (mv->move == B->state->killer[0]) mv->val = O_KILLER1;
            else if (mv->move == B->state->killer[1]) mv->val = O_KILLER2;
            else mv->val = MAX(0, B->history[B->sq[from]][to]); // Quiet move
        }
    }
    //sort(moves, last);

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

#define ADD_MOVE(from, to, flags) { (last++)->move = MOVE(from, to, flags); }
#define ADD_QUIET(from, to)         ADD_MOVE(from, to, 0);
#define ADD_CAP(from, to)           ADD_MOVE(from, to, F_CAP);
#define ADD_PROM(from, to)        { ADD_MOVE(from, to, F_QPROM); \
                                    ADD_MOVE(from, to, F_RPROM); \
                                    ADD_MOVE(from, to, F_NPROM); \
                                    ADD_MOVE(from, to, F_BPROM); }
#define ADD_CAPPROM(from, to)     { ADD_MOVE(from, to, F_QCAPPROM); \
                                    ADD_MOVE(from, to, F_RCAPPROM); \
                                    ADD_MOVE(from, to, F_NCAPPROM); \
                                    ADD_MOVE(from, to, F_BCAPPROM); }

// depth 6 - 13000 knps (pseudolegal | x64)
// depth 6 - 10700 knps (pseudolegal | x64) update_tactics + NBRQ

void MoveList::generate_all()
{
	U64 own = B->occ[B->wtm];
	U64 opp = B->occ[B->wtm ^ 1];
	U64 o = own | opp, bit;
	int p, to;

	ASSERT(!(opp & own));

	// King /////////////////////////////////////////////////////////////////////////////////////

	int ksq = BITSCAN(B->piece[BK + B->wtm]);
	for (U64 att = pieces[BK + B->wtm].att[ksq] & ~own; att; RLSB(att))
	{
		bit = LSB(att);
		to = BITSCAN(bit);
		ADD_MOVE(ksq, to, (bit & opp) ? F_CAP : 0);
	}

	// Castling /////////////////////////////////////////////////////////////////////////////////

    if (!B->state->checks)
    {
        if (B->wtm)
        {
            if (B->state->castling & C_WK)
            {
                if (!(o & (sF1 | sG1)))
                    ADD_MOVE(E1, G1, F_KCASTLE);
            }

            if (B->state->castling & C_WQ)
            {
                if (!(o & (sB1 | sC1 | sD1)))
                    ADD_MOVE(E1, C1, F_QCASTLE);
            }
        }
        else
        {
            if (B->state->castling & C_BK)
            {
                if (!(o & (sF8 | sG8)))
                    ADD_MOVE(E8, G8, F_KCASTLE);
            }

            if (B->state->castling & C_BQ)
            {
                if (!(o & (sB8 | sC8 | sD8)))
                    ADD_MOVE(E8, C8, F_QCASTLE);
            }
        }
    }

	// Pawns ////////////////////////////////////////////////////////////////////////////////////

	p = BP + B->wtm;
	if (B->wtm)
	{
		if (B->piece[p])
		{
			// Promotion & move forward
			for (U64 bb = B->piece[p] & SHIFT__D(~o); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 6) ADD_PROM(s, s + 8)
				else           ADD_QUIET(s, s + 8);
			}

			// Attacks
			for (U64 bb = B->piece[p] & SHIFT_DL(opp); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 6) ADD_CAPPROM(s, s + 7)
				else           ADD_CAP(s, s + 7);
			}

			for (U64 bb = B->piece[p] & SHIFT_DR(opp); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 6) ADD_CAPPROM(s, s + 9)
				else           ADD_CAP(s, s + 9);
			}

			// Double move
			for (U64 bb = B->piece[p] & ((~o) >> 8) & ((~o) >> 16); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 1) ADD_MOVE(s, s + 16, F_PAWN2);
			}

			// En passant
			if (B->state->ep)
			{
				for (U64 bb = B->piece[p] & pieces[B->wtm ^ 1].att[B->state->ep]; bb; RLSB(bb))
				{
					int s = BITSCAN(bb);
					ADD_MOVE(s, B->state->ep, F_EP);
				}
			}
		}
	}
	else
	{
		if (B->piece[p])
		{
			// Promotion & move forward
			for (U64 bb = B->piece[p] & SHIFT__U(~o); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 1) ADD_PROM(s, s - 8)
				else           ADD_QUIET(s, s - 8);
			}

			// Attacks
			for (U64 bb = B->piece[p] & SHIFT_UR(opp); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 1) ADD_CAPPROM(s, s - 7)
				else           ADD_CAP(s, s - 7);
			}

			for (U64 bb = B->piece[p] & SHIFT_UL(opp); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 1) ADD_CAPPROM(s, s - 9)
				else           ADD_CAP(s, s - 9);
			}

			// Double move
			for (U64 bb = B->piece[p] & ((~o) << 8) & ((~o) << 16); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 6) ADD_MOVE(s, s - 16, F_PAWN2);
			}

			// En passant
			if (B->state->ep)
			{
				for (U64 bb = B->piece[p] & pieces[B->wtm ^ 1].att[B->state->ep]; bb; RLSB(bb))
				{
					int s = BITSCAN(bb);
					ADD_MOVE(s, B->state->ep, F_EP);
				}
			}
		}
	}

	// Knights //////////////////////////////////////////////////////////////////////////////////

	p = BN + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
		for (U64 att = pieces[p].att[s] & opp; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));

        for (U64 att = pieces[p].att[s] & ~o; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

	// Bishops //////////////////////////////////////////////////////////////////////////////////

	p = BB + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = batt(s, o);

		for (U64 att = a & opp; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));

        for (U64 att = a & ~o; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = batt(s, o);

		for (U64 att = a & opp & B->state->pins[s]; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));

        for (U64 att = a & ~o & B->state->pins[s]; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

	// Rooks ////////////////////////////////////////////////////////////////////////////////////

	p = BR + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = ratt(s, o);

		for (U64 att = a & opp; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));

        for (U64 att = a & ~o; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = ratt(s, o);

		for (U64 att = a & opp & B->state->pins[s]; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));

        for (U64 att = a & ~o & B->state->pins[s]; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

	// Queens ///////////////////////////////////////////////////////////////////////////////////

	p = BQ + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = qatt(s, o);

		for (U64 att = a & opp; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));

        for (U64 att = a & ~o; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = qatt(s, o);

		for (U64 att = a & opp & B->state->pins[s]; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));

        for (U64 att = a & ~o & B->state->pins[s]; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}
}

void MoveList::generate_attacks()
{
	U64 own = B->occ[B->wtm];
	U64 opp = B->occ[B->wtm ^ 1];
	U64 o = own | opp, bit;
	int p, to;

	ASSERT(!(opp & own));

	// King /////////////////////////////////////////////////////////////////////////////////////

	int ksq = BITSCAN(B->piece[BK + B->wtm]);
	for (U64 att = pieces[BK + B->wtm].att[ksq] & opp; att; RLSB(att))
	{
		bit = LSB(att);
		to = BITSCAN(bit);
		ADD_CAP(ksq, to);
	}

	// Pawns ////////////////////////////////////////////////////////////////////////////////////

	p = BP + B->wtm;
	if (B->wtm)
	{
		if (B->piece[p])
		{
			// Promotion & move forward
			for (U64 bb = B->piece[p] & SHIFT__D(~o) & RANK_7; bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				ADD_PROM(s, s + 8);
			}

			// Attacks
			for (U64 bb = B->piece[p] & SHIFT_DL(opp); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 6) ADD_CAPPROM(s, s + 7)
				else           ADD_CAP(s, s + 7);
			}

			for (U64 bb = B->piece[p] & SHIFT_DR(opp); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 6) ADD_CAPPROM(s, s + 9)
				else           ADD_CAP(s, s + 9);
			}

			// En passant
			if (B->state->ep)
			{
				for (U64 bb = B->piece[p] & pieces[B->wtm ^ 1].att[B->state->ep]; bb; RLSB(bb))
				{
					int s = BITSCAN(bb);
					ADD_MOVE(s, B->state->ep, F_EP);
				}
			}
		}
	}
	else
	{
		if (B->piece[p])
		{
			// Promotion & move forward
			for (U64 bb = B->piece[p] & SHIFT__U(~o) & RANK_2; bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				ADD_PROM(s, s - 8);
			}

			// Attacks
			for (U64 bb = B->piece[p] & SHIFT_UR(opp); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 1) ADD_CAPPROM(s, s - 7)
				else           ADD_CAP(s, s - 7);
			}

			for (U64 bb = B->piece[p] & SHIFT_UL(opp); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 1) ADD_CAPPROM(s, s - 9)
				else           ADD_CAP(s, s - 9);
			}

			// En passant
			if (B->state->ep)
			{
				for (U64 bb = B->piece[p] & pieces[B->wtm ^ 1].att[B->state->ep]; bb; RLSB(bb))
				{
					int s = BITSCAN(bb);
					ADD_MOVE(s, B->state->ep, F_EP);
				}
			}
		}
	}

	// Knights //////////////////////////////////////////////////////////////////////////////////

	p = BN + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
		for (U64 att = pieces[p].att[s] & opp; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));
	}

	// Bishops //////////////////////////////////////////////////////////////////////////////////

	p = BB + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = batt(s, o);

		for (U64 att = a & opp; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = batt(s, o);

		for (U64 att = a & opp & B->state->pins[s]; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));
	}

	// Rooks ////////////////////////////////////////////////////////////////////////////////////

	p = BR + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = ratt(s, o);

		for (U64 att = a & opp; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = ratt(s, o);

		for (U64 att = a & opp & B->state->pins[s]; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));
	}

	// Queens ///////////////////////////////////////////////////////////////////////////////////

	p = BQ + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = qatt(s, o);

		for (U64 att = a & opp; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        U64 a = qatt(s, o);

		for (U64 att = a & opp & B->state->pins[s]; att; RLSB(att))
            ADD_CAP(s, BITSCAN(att));
	}
}

void MoveList::generate_quiets()
{
    U64 own = B->occ[B->wtm];
	U64 opp = B->occ[B->wtm ^ 1];
	U64 o = own | opp, bit;
	int p, to;

	ASSERT(!(opp & own));

	// King /////////////////////////////////////////////////////////////////////////////////////

	int ksq = BITSCAN(B->piece[BK + B->wtm]);
	for (U64 att = pieces[BK + B->wtm].att[ksq] & ~o; att; RLSB(att))
	{
		bit = LSB(att);
		to = BITSCAN(bit);
		ADD_QUIET(ksq, to);
	}

	// Castling /////////////////////////////////////////////////////////////////////////////////

    if (!B->state->checks)
    {
        if (B->wtm)
        {
            if (B->state->castling & C_WK)
            {
                if (!(o & (sF1 | sG1)))
                    ADD_MOVE(E1, G1, F_KCASTLE);
            }

            if (B->state->castling & C_WQ)
            {
                if (!(o & (sB1 | sC1 | sD1)))
                    ADD_MOVE(E1, C1, F_QCASTLE);
            }
        }
        else
        {
            if (B->state->castling & C_BK)
            {
                if (!(o & (sF8 | sG8)))
                    ADD_MOVE(E8, G8, F_KCASTLE);
            }

            if (B->state->castling & C_BQ)
            {
                if (!(o & (sB8 | sC8 | sD8)))
                    ADD_MOVE(E8, C8, F_QCASTLE);
            }
        }
    }

	// Pawns ////////////////////////////////////////////////////////////////////////////////////

	p = BP + B->wtm;
	if (B->wtm)
	{
		if (B->piece[p])
		{
			// Move forward
			for (U64 bb = B->piece[p] & SHIFT__D(~o); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) != 6) ADD_QUIET(s, s + 8);
			}

			// Double move
			for (U64 bb = B->piece[p] & ((~o) >> 8) & ((~o) >> 16); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 1) ADD_MOVE(s, s + 16, F_PAWN2);
			}
		}
	}
	else
	{
		if (B->piece[p])
		{
			// Move forward
			for (U64 bb = B->piece[p] & SHIFT__U(~o); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) != 1) ADD_QUIET(s, s - 8);
			}

			// Double move
			for (U64 bb = B->piece[p] & ((~o) << 8) & ((~o) << 16); bb; RLSB(bb))
			{
				int s = BITSCAN(bb);
				if (Y(s) == 6) ADD_MOVE(s, s - 16, F_PAWN2);
			}
		}
	}

	// Knights //////////////////////////////////////////////////////////////////////////////////

	p = BN + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        for (U64 att = pieces[p].att[s] & ~o; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

	// Bishops //////////////////////////////////////////////////////////////////////////////////

	p = BB + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        for (U64 att = batt(s, o) & ~o; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        for (U64 att = batt(s, o) & ~o & B->state->pins[s]; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

	// Rooks ////////////////////////////////////////////////////////////////////////////////////

	p = BR + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        for (U64 att = ratt(s, o) & ~o; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        for (U64 att = ratt(s, o) & ~o & B->state->pins[s]; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

	// Queens ///////////////////////////////////////////////////////////////////////////////////

	p = BQ + B->wtm;
	for (U64 bb = B->piece[p] & ~B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        for (U64 att = qatt(s, o) & ~o; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}

    for (U64 bb = B->piece[p] & B->state->pinned; bb; RLSB(bb))
	{
		int s = BITSCAN(bb);
        for (U64 att = qatt(s, o) & ~o & B->state->pins[s]; att; RLSB(att))
            ADD_QUIET(s, BITSCAN(att));
	}
}

void MoveList::generate_evasions() // Not necessarily actually
{
    // 1. Capture attacker ////////////////////////////////////

    int to = BITSCAN(B->state->checkers);
    for (U64 att = B->get_attackers(to, B->wtm); att; RLSB(att))
        ADD_CAP(BITSCAN(att), to);

    if (B->state->checks < 2)
    {
        // 2. Block attack ////////////////////////////////////


    }
}

void MoveList::sort()
{
    std::sort(first, last);
}

