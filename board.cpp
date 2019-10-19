#include <sstream>
#include <string>
#include "board.h"
#include "search.h"
#include "magics.h"
#include "piece.h"
#include "util.h"
#include "hash.h"
#include "eval.h"

using namespace std;

void Board::clear()
{
    wtm = 1;
    for (int i = 0; i < PIECE_N; i++) piece[i] = EMPTY;
    for (int i = 0; i < COLOR_N; i++) occ[i] = EMPTY;
    for (int i = 0; i < SQUARE_N; i++) sq[i] = NOP;
}

void Board::reset()
{
    from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::print()
{
    const string pieceChar = "pPnNbBrRqQkK..";
    const string toMove = wtm ? "<W>" : "<B>";
    U64 occupied = occ[0] | occ[1];

    for (int y = 7; y >= 0; y--)
    {
        CON(RANKS[y] << "|");

        for (int x = 0; x < 8; x++)
        {
            int s = SQ(x, y);
            U64 bit = BIT << s;
            CON((bit & occupied ? pieceChar[sq[s]] : '.'));
        }

        if (y == 0) CON(" " << toMove);
        CON("\n");
    }
    CON(" +--------\n  " << FILES << "\n\n");
}

#define SET_PIECE(s, p) { piece[p] |= (BIT << (s));   \
                          sq[s] = (p);                \
                          state->pst += E->pst[p][s]; \
                                                      }

void Board::from_fen(const char * fen)
{
    clear();
    state->pst.clear();
    int i = 0, square = A8;

    while (char ch = fen[i++])
    {
        bool shift = true;
        switch(ch)
        {
            case 'p': SET_PIECE(square, BP); break;
            case 'P': SET_PIECE(square, WP); break;
            case 'n': SET_PIECE(square, BN); break;
            case 'N': SET_PIECE(square, WN); break;
            case 'b': SET_PIECE(square, BB); break;
            case 'B': SET_PIECE(square, WB); break;
            case 'r': SET_PIECE(square, BR); break;
            case 'R': SET_PIECE(square, WR); break;
            case 'q': SET_PIECE(square, BQ); break;
            case 'Q': SET_PIECE(square, WQ); break;
            case 'k': SET_PIECE(square, BK); break;
            case 'K': SET_PIECE(square, WK); break;

            case '/': continue;
            case ' ': square = -1; break;

            default:

                shift = false;
                if (ch > '0' && ch < '9') square += ch - '0';
        }

        if (shift) square++;

        if (!(square & 7)) // End of row
        {
            square -= 16;
            if (square < 0) break;
        }
    }

    // White to move
    wtm = 2;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == 'w') wtm = WHITE;
        if (ch == 'b') wtm = BLACK;
        if (wtm < 2) break;
    }

    // Castling rights
    state->castling = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == '-' || ch == ' ') break;
        if (ch == 'k') state->castling |= C_BK;
        if (ch == 'q') state->castling |= C_BQ;
        if (ch == 'K') state->castling |= C_WK;
        if (ch == 'Q') state->castling |= C_WQ;
    }

    // En passant target square
    state->ep = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == '-' || ch == ' ') break;
        if (ch >= 'a' && ch <= 'h') state->ep += SQ(ch - 'a', 0);
        if (ch >= 'A' && ch <= 'H') state->ep += SQ(ch - 'A', 0);
        if (ch == '3' || ch == '6') state->ep += SQ(0, ch - '1');
    }
    if (Y(state->ep) != 2 && Y(state->ep) != 5) state->ep = 0;

    // Halfmove clock
    state->fifty = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == ' ') break;
        if (ch >= '0' && ch <= '9')
        {
            state->fifty *= 10;
            state->fifty += ch - '0';
        }
    }

    // Full move counter
    S->movecnt = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == ' ') break;
        if (ch >= '0' && ch <= '9')
        {
            S->movecnt *= 10;
            S->movecnt += ch - '0';
        }
    }
    S->movecnt *= 2;
    S->movecnt -= wtm;
    S->movecnt--;

    // Occupied
    occ[0] = occ[1] = EMPTY;
    for (int i = BP; i < PIECE_N; i += 2)
    {
        occ[0] |= piece[i];
        occ[1] |= piece[i + 1];
    }

    state->hash = calc_hash();
    /*
    N->phash = calcPHash(N);
    S->threefold[S->movecnt] = N->hash;*/

    update_mat_pst();
    update_tactics();
}

string Board::make_fen()
{
	stringstream ss;
	const char * pieces = "pPnNbBrRqQkK.";
    char board[SQUARE_N] = { ' ', };

	for (int i = 0; i < PIECE_N; i++)
	{
		for (U64 bb = piece[i]; bb; bb = RLSB(bb))
		{
			int j = BITSCAN(bb);
			board[j] = pieces[i];
		}
	}

	for (int y = 7; y >= 0; y--)
	{
		int empty = 0;
		for (int x = 0; x < 8; x++)
		{
			int j = SQ(x, y);
			char ch = board[j];

			if (ch == ' ')
			{
				empty++;
				if (x == 7) ss << empty;
			}
			else
			{
				if (empty) ss << empty;
				empty = 0;
				ss << ch;
			}
		}
		if (y) ss << '/';
	}

	ss << " " << "bw"[wtm & 1] << " - - ";
	ss << state->fifty << " ";
	ss << state->fifty / 2;

	return ss.str();
}

U64 Board::calc_hash()
{
	U64 hash = EMPTY;

	for (int i = 0; i < 12; i++)
	{
		for (U64 bb = piece[i]; bb; bb = RLSB(bb))
		{
			int j = BITSCAN(bb);
			hash ^= hash_key[i][j];
		}
	}

	hash ^= hash_castle[state->castling];
	hash ^= hash_ep[state->ep];
	if (wtm) hash ^= hash_wtm;

	return hash;
}

bool Board::try_parse(string str, Move & move)
{
    move = MOVE_NONE;
	if (str[0] < 'a' || str[0] > 'h') return false;
	if (str[1] < '1' || str[1] > '8') return false;
	if (str[2] < 'a' || str[2] > 'h') return false;
	if (str[3] < '1' || str[3] > '8') return false;

	int from  = SQ(str[0] - 'a', str[1] - '1');
	int to    = SQ(str[2] - 'a', str[3] - '1');
	int flags = occ[wtm ^ 1] & (BIT << to) ? F_CAP : 0;
	int p     = sq[from];

	switch (str[4])
	{
		case 'n': flags += F_NPROM; move = MOVE(from, to, flags); return true;
		case 'b': flags += F_BPROM; move = MOVE(from, to, flags); return true;
		case 'r': flags += F_RPROM; move = MOVE(from, to, flags); return true;
		case 'q': flags += F_QPROM; move = MOVE(from, to, flags); return true;
	}

	// Recognizing castling

	if (p == WK)
	{
		if (from == E1)
		{
			if (to == G1) flags = F_KCASTLE;
			if (to == C1) flags = F_QCASTLE;
		}
	}
	else if (p == BK)
	{
		if (from == E8)
		{
			if (to == G8) flags = F_KCASTLE;
			if (to == C8) flags = F_QCASTLE;
		}
	}
	else if (p == WP && Y(from) == 1 && Y(to) == 3)
	{
		flags = F_PAWN2;
	}
	else if (p == BP && Y(from) == 6 && Y(to) == 4)
	{
		flags = F_PAWN2;
	}
	else if (state->ep && p < BN)
	{
		if (Y(state->ep) == 2 || Y(state->ep) == 5)
		if (to == state->ep) flags = F_EP; // Recognizing en passant
	}

    move = MOVE(from, to, flags);
    string smove = to_string(move);

	return is_allowed(move);
}

bool Board::is_allowed(Move move) // Pseudolegality
{
    int flags = FLAGS(move);
    int from = FROM(move);
    int to = TO(move);
    int p = sq[from];

	// 1. Our piece must be on from square
	if (!(occ[wtm] & (BIT << from))) return false;

	// 2. This piece must be able to move in such way
	bool pawnmove = false;

	// 2.1. Checking castling
	if (IS_CASTLE(flags))
	{
		if (p == WK)
		{
			if (from != E1) return false;
			if (to != G1 && to != C1) return false;
			return true;
		}
		else if (p == BK)
		{
			if (from != E8) return false;
			if (to != G8 && to != C8) return false;
			return true;
		}
		else return false;
	}

	if (p < BN) // pawn
	{
		if (!((BIT << to) & pieces[p].mov[from])) // not move forward
		{
			if (!((BIT << to) & pieces[p].att[from])) return false; // not attack
			if (to != state->ep) // not en passant
			{
				if (!((BIT << to) & occ[wtm ^ 1])) return false; // not capturing
			}
		}
	}
	else // not pawn
	{
		if (!((BIT << to) & pieces[p].att[from])) return false; // not correct move
	}
	
	// 3. There are no pieces on a way of piece
	if (between[from][to] & (occ[0] | occ[1])) return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

bool Board::is_attacked(int ksq, U64 occupied, int opposite) // by opponent (default)
{
    int col = wtm ^ opposite;
    if (pieces[WN ^ col].att[ksq] & piece[BN ^ col]) return true; // Knights
	if (pieces[WP ^ col].att[ksq] & piece[BP ^ col]) return true; // Pawns
	if (pieces[WK ^ col].att[ksq] & piece[BK ^ col]) return true; // King

	if (BATT(ksq, occupied) & ((piece[BB ^ col] | piece[BQ ^ col]))) return true; // Bishops & queens
	if (RATT(ksq, occupied) & ((piece[BR ^ col] | piece[BQ ^ col]))) return true; // Rooks & queens

	return false;
}

// Assuming we are not in check
bool Board::is_pinned(int ksq, U64 occupied, U64 captured, U64 & att)
{
	U64 bq = (piece[WB ^ wtm] | piece[WQ ^ wtm]) & ~captured;
	U64 rq = (piece[WR ^ wtm] | piece[WQ ^ wtm]) & ~captured;

	// Bishops & queens
	if (pieces[BB ^ wtm].att[ksq] & bq)
	if (att = BATT(ksq, occupied) & bq) return true;

	// Rooks & queens
	if (pieces[BR ^ wtm].att[ksq] & rq)
	if (att = RATT(ksq, occupied) & rq) return true;

	return false;
}

int Board::cnt_attacks(int ksq, U64 occupied, U64 captured, U64 & att)
{
	int cnt = 0;
	att = EMPTY;
	U64 bb;

	if (bb = pieces[BK ^ wtm].att[ksq] & piece[WK ^ wtm]) {cnt++; att |= bb;} // King
	if (bb = pieces[BP ^ wtm].att[ksq] & piece[WP ^ wtm]) {cnt++; att |= bb;} // Pawns
	if (bb = pieces[BN ^ wtm].att[ksq] & piece[WN ^ wtm]) {cnt++; att |= bb;} // Knights

	if (cnt > 1) return cnt;

	U64 bq = (piece[WB ^ wtm] | piece[WQ ^ wtm]) & ~captured;
	U64 rq = (piece[WR ^ wtm] | piece[WQ ^ wtm]) & ~captured;

	// Bishops & queens
	if (pieces[BB ^ wtm].att[ksq] & bq)
	if (bb = BATT(ksq, occupied) & bq) {cnt++; att |= bb;}

	if (cnt > 1) return cnt;

	// Rooks & queens
	if (pieces[BR ^ wtm].att[ksq] & rq)
	if (bb = RATT(ksq, occupied) & rq) {cnt++; att |= bb;}

	return cnt;
}

U64 Board::get_attacks_xray(int sq, U64 occupied, int col)
{
	U64 bb, att = EMPTY;
	if (col == 2) col = wtm;

	att |= pieces[BK ^ col].att[sq] & piece[BK ^ col];
	att |= pieces[BP ^ col].att[sq] & piece[BP ^ col];
	att |= pieces[BN ^ col].att[sq] & piece[BN ^ col];

	U64 bq = piece[BB ^ col] | piece[BQ ^ col];
	U64 rq = piece[BR ^ col] | piece[BQ ^ col];

	// Bishops & queens (xray)
	while (true)
	{
		bb = BATT(sq, occupied) & bq;
		att |= bb;

		if (!(bb & bq & occupied)) break;
		occupied &= ~(bb & bq);
	}

	// Rooks & queens (xray)
	while (true)
	{
		bb = RATT(sq, occupied) & rq;
		att |= bb;

		if (!(bb & rq & occupied)) break;
		occupied &= ~(bb & rq);
	}

	return att;
}

inline bool Board::in_check() // In current position
{
	return is_attacked(BITSCAN(piece[BK + wtm]), occ[0] | occ[1]);
}

U64 Board::get_attack(int piece, int sq)
{
    U64 occupied, att = EMPTY;

    switch (TYPE(piece))
    {
        case BISHOP:
            occupied = occ[0] | occ[1];
            att = BATT(sq, occupied);
            break;

        case ROOK:
            occupied = occ[0] | occ[1];
            att = RATT(sq, occupied);
            break;

        case QUEEN:
            occupied = occ[0] | occ[1];
            att = QATT(sq, occupied);
            break;

        default:
            att = pieces[piece].att[sq];
    }

    return att;
}

///////////////////////////////////////////////////////////////////////////////

void Board::update_killers(Move move, int depth)
{
    /*if (move != state->killer[0])
    {
        state->killer[1] = state->killer[0];
        state->killer[0] = move;
    }*/

    if ((history[sq[FROM(move)]][TO(move)] += depth * depth) >= 60000)
	{
		for (int i = 0; i < 12; i++)
            for (int j = 0; j < 64; j++)
                history[i][j] >>= 1;
	}
}

void Board::update_mat_pst()
{
    state->pst.clear();
    state->mkey = EMPTY;

    for (int i = 0; i < PIECE_N; i++)
	{
        state->mkey += matkey[i] * POPCNT(B->piece[i]);
		for (U64 bb = B->piece[i]; bb; bb = RLSB(bb))
		{
			int sq = BITSCAN(bb);
			state->pst += E->pst[i][sq];
		}
	}
}

void Board::update_tactics()
{
    state->checks = 0;
    state->checkers = EMPTY;
    state->pinned = EMPTY;

    U64 bb, o = occ[0] | occ[1];
    int ksq = BITSCAN(piece[BK + wtm]);

    if (bb = pieces[BN ^ wtm].att[ksq] & piece[WN ^ wtm]) { state->checks++; state->checkers |= bb; }; // Knights
	if (bb = pieces[BP ^ wtm].att[ksq] & piece[WP ^ wtm]) { state->checks++; state->checkers |= bb; }; // Pawns
	if (bb = pieces[BK ^ wtm].att[ksq] & piece[WK ^ wtm]) { state->checks++; state->checkers |= bb; }; // King

    for (bb = pieces[BB ^ wtm].att[ksq] & ((piece[WB ^ wtm] | piece[WQ ^ wtm])); bb; RLSB(bb)) // Bishops & queens
    {
        int j = BITSCAN(bb);
        U64 b = between[ksq][j] & o;
        if (!b) { state->checks++; state->checkers |= BIT << j; }
        else if (!LSB(b))
        {
            state->pinned |= b;
            state->pins[BITSCAN(b)] = between[ksq][j];
        }
    }

    for (bb = pieces[BR ^ wtm].att[ksq] & ((piece[WR ^ wtm] | piece[WQ ^ wtm])); bb; RLSB(bb)) // Rooks & queens
    {
        int j = BITSCAN(bb);
        U64 b = between[ksq][j] & o;
        if (!b) { state->checks++; state->checkers |= BIT << j; }
        else if (!LSB(b))
        {
            state->pinned |= b;
            state->pins[BITSCAN(b)] = between[ksq][j];
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

bool Board::make(int move, bool self)
{
    ASSERT_MOVE(B->is_allowed(Move(move)), Move(move));

    int flags = FLAGS(move);
    int from = FROM(move);
    int to = TO(move);
    int p = sq[from];

    ASSERT(p != NOP)
	ASSERT(IS_VALID(move));

    if (!self)
    {
        state++;
        state->hash = (state - 1)->hash
                    ^ hash_ep[(state - 1)->ep]
                    ^ hash_castle[(state - 1)->castling]; // Reset flags
        state->fifty = (state - 1)->fifty;
        state->castling = (state - 1)->castling;

        state->mkey = (state - 1)->mkey;
        state->pst = (state - 1)->pst;
    }

	state->castling &= uncastle[from] & uncastle[to];
	state->cap = 0;
	state->ep = 0;
    state->fifty++;
	int prom, cap;

    //NN->canNull = N->canNull;

	switch(flags)
	{
		case F_CAP:

			state->cap = sq[to];

            remove(from);
            remove(to);
            place(to, p);
            
			state->fifty = 0;
			break;

		case F_KCASTLE:

            remove(from);
            remove(to + 1);
            place(to, p);
            place(to - 1, BR + wtm);

			state->fifty = 0;
			break;

		case F_QCASTLE:

            remove(from);
            remove(to - 2);
            place(to, p);
            place(to + 1, BR + wtm);

			state->fifty = 0;
			break;

		case F_NPROM: case F_BPROM: case F_RPROM: case F_QPROM:

			prom = 2 * (flags - F_NPROM) + BN + wtm;

            remove(from);
            place(to, prom);

			state->fifty = 0;
			break;

		case F_NCAPPROM: case F_BCAPPROM: case F_RCAPPROM: case F_QCAPPROM:

			prom = 2 * (flags - F_NCAPPROM) + BN + wtm;
			state->cap = sq[to];

            remove(from);
            remove(to);
            place(to, prom);

			state->fifty = 0;
			break;

		case F_EP:

			cap = SQ(X(to), Y(from));

            remove(cap);
            remove(from);
            place(to, p);

			state->fifty = 0;
			break;

		case F_PAWN2: // ---------- FALL THROUGH! ------------

			state->ep = (from + to) / 2;
			state->fifty = 0;

		default:

            remove(from);
            place(to, p);

			if (IS_PAWN(p)) state->fifty = 0;
	}

	state->hash ^= hash_ep[state->ep] ^ hash_castle[state->castling] ^ hash_wtm; // Set flags
	wtm ^= 1;

    //U64 hash = calc_hash();
    //assert(state->hash == hash);

    S->movecnt++;
	bool reversible = !flags && (p > WP);
    //S->threefold[S->movecnt] = reversible ? state->hash : EMPTY;

    // Legality check //////////////////////////////////////////////////////////////////////////

    U64 o = occ[0] | occ[1];
    if (is_attacked(BITSCAN(piece[BK + wtm ^ 1]), o))
    {
        unmake(move);
        return false;
    }

    switch (flags)
    {
        case F_KCASTLE:
            
            if (wtm) // color changed
            {
                if (is_attacked(G8, o) || is_attacked(F8, o))
                {
                    unmake(move);
                    return false;
                }
            }
            else
            {
                if (is_attacked(G1, o) || is_attacked(F1, o))
                {
                    unmake(move);
                    return false;
                }
            }
            break;

        case F_QCASTLE:
            
            if (wtm) // color changed
            {
                if (is_attacked(C8, o) || is_attacked(D8, o))
                {
                    unmake(move);
                    return false;
                }
            }
            else
            {
                if (is_attacked(C1, o) || is_attacked(D1, o))
                {
                    unmake(move);
                    return false;
                }
            }
            break;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    update_tactics();

    ASSERT( !(occ[0] & occ[1]) );
    return true;
}

void Board::unmake(int move)
{
    ASSERT(IS_VALID(move));

    int flags = FLAGS(move);
    int from = FROM(move);
    int to = TO(move);
    int p = sq[to];

    //S->threefold[S->movecnt] = NN->hash;
    S->movecnt--;
	wtm ^= 1;
	int prom, cap;

	switch (flags)
	{
		case F_CAP: 

            remove<false>(to);
            place<false>(from, p);
            place<false>(to, state->cap);
			break;

		case F_KCASTLE:

            remove<false>(to - 1);
            remove<false>(to);
            place<false>(to + 1, BR + wtm);
            place<false>(from, p);
			break;

		case F_QCASTLE:

            remove<false>(to + 1);
            remove<false>(to);
            place<false>(to - 2, BR + wtm);
            place<false>(from, p);
			break;

		case F_NPROM: case F_BPROM: case F_RPROM: case F_QPROM:

			prom = 2 * (flags - F_NPROM) + BN + wtm;

            remove<false>(to);
            place<false>(from, BP + wtm);
			break;

		case F_NCAPPROM: case F_BCAPPROM: case F_RCAPPROM: case F_QCAPPROM:

			prom = 2 * (flags - F_NCAPPROM) + BN + wtm;

            remove<false>(to);
            place<false>(from, BP + wtm);
            place<false>(to, state->cap);
			break;

		case F_EP:

			cap = SQ(X(to), Y(from));

            remove<false>(to);
            place<false>(cap, p ^ 1);
            place<false>(from, p);
			break;

		default:

            remove<false>(to);
            place<false>(from, p);
	}

    //NN->canNull = true;
    state->cap = NOP;
    state->ep = 0;
	state--;

    ASSERT( !(occ[0] & occ[1]) );
}
