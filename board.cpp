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
    U64 occupied = occ[BLACK] | occ[WHITE];

    for (int y = 7; y >= 0; y--)
    {
        CON(RANKS[y] << "|");

        for (int x = 0; x < 8; x++)
        {
            U64 bit = BIT << SQ(x, y);

            if (bit & occupied)
            {
                char ch = '?';
                int color = !!(bit & occ[BLACK]);
                for (int k = BP + color; k < PIECE_N; k += 2)
                {
                    if (bit & piece[k])
                    {
                        ch = pieceChar[k];
                        break;
                    }
                }
                CON(ch);
            }
            else
                CON('.');
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
    state->pst = 0;
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
        if (ch == 'w') wtm = 1;
        if (ch == 'b') wtm = 0;
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

    // Full move counter -> TODO
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
			hash ^= hashKey[i][j];
		}
	}

	hash ^= hashCastle[state->castling];
	hash ^= hashWtm[wtm];
	hash ^= hashEp[state->ep];

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
    move = MOVE(from, to, flags);

	if (!IS_VALID(move)) return false;

	switch (str[4])
	{
		case 'n': flags += F_NPROM; return true;
		case 'b': flags += F_BPROM; return true;
		case 'r': flags += F_RPROM; return true;
		case 'q': flags += F_QPROM; return true;
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

bool Board::is_legal(Move move)
{
    int flags = FLAGS(move);
    int from = FROM(move);
    int to = TO(move);
    int p = sq[from];

	U64 att, occupied = (occ[0] | occ[1] | (BIT << to)) & ~(BIT << from);
	U64 cap = IS_CAP(flags) ? (BIT << to) : EMPTY;
	int ksq = BITSCAN(piece[BK + wtm]);
	int checks = cnt_attacks(ksq, occupied, cap, att);

	if (checks > 1) // Double check
	{
		// 0. No castling
		if (IS_CASTLE(flags)) return false;

		// 1. Evasions
		if (p == BK || p == WK)
		{
			if (is_attacked(to, occupied, cap))
				return false;
		}
		else return false;
	}
	else if (checks == 1) // Check
	{
		// 0. No castling
		if (IS_CASTLE(flags)) return false;

		// 1. Evasions
		if (p == BK || p == WK)
		{
			if (is_attacked(to, occupied, cap))
				return false;
		}
		else
		{
			// 2. Capture attacker
			if ((BIT << to) & att) return true;

			// 2.1. Capture pawn by en-passant
			if (IS_EP(flags))
			{
				int ep = SQ( X(to), Y(from) );
				if ((BIT << ep) & att) return true;
			}

			// 3. Block attacker
			if ((BIT << to) & between[ksq][BITSCAN(att)]) return true;

			return false;
		}
	}
	else // Non-check
	{
		if (p == BK || p == WK)
		{
			if (is_attacked(to, occupied, cap)) // (1)
				return false;

			if (IS_CASTLE(flags)) // (2)
			{
				if (is_attacked((from + to) / 2, occupied, cap)) return false;
			}
		}
		else // (3)
		{
			if (IS_EP(flags)) occupied &= ~(BIT << SQ( X(state->ep), Y(from) )); // En passant
			if (is_pinned(ksq, occupied, cap, att)) return false;

			// 2. Capture attacker
			//if ((BIT << to) & att) return true;

			// 3. Block attacker
			//if ((BIT << to) & between[ksq][BITSCAN(att)]) return true;

			//return false;
			return true;
		}
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////

bool Board::is_attacked(int ksq, U64 occupied, U64 captured)
{
	if (pieces[BK ^ wtm].att[ksq] & piece[WK ^ wtm]) return true; // King
	if (pieces[BP ^ wtm].att[ksq] & piece[WP ^ wtm]) return true; // Pawns
	if (pieces[BN ^ wtm].att[ksq] & piece[WN ^ wtm]) return true; // Knights

	U64 bq = (piece[WB ^ wtm] | piece[WQ ^ wtm]) & ~captured;
	U64 rq = (piece[WR ^ wtm] | piece[WQ ^ wtm]) & ~captured;

	// Bishops & queens
	//if (pieces[BB ^ wtm].att[ksq] & bq)
	if (BATT(ksq, occupied) & bq) return true;

	// Rooks & queens
	//if (pieces[BR ^ wtm].att[ksq] & rq)
	if (RATT(ksq, occupied) & rq) return true;

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

bool Board::in_check()
{
	return is_attacked( BITSCAN(piece[BK + wtm]), occ[0] | occ[1], EMPTY );
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

void Board::make(int move, bool self)
{
    int flags = FLAGS(move);
    int from = FROM(move);
    int to = TO(move);
    int p = piece[to];

	ASSERT(IS_VALID(move));

    if (!self)
    {
        state++;
        state->hash = (state - 1)->hash
                    ^ hashEp[(state - 1)->ep]
                    ^ hashCastle[(state - 1)->castling]; // Reset flags
        state->fifty = (state - 1)->fifty;
        state->castling = (state - 1)->castling;
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

	state->hash ^= hashEp[state->ep] ^ hashCastle[state->castling] ^ hashWtm[0]; // Set flags

	wtm ^= 1;

    S->movecnt++;
	bool reversible = !flags && (p > WP);
    S->threefold[S->movecnt] = reversible ? state->hash : EMPTY;

    ASSERT( !(occ[0] & occ[1]) );
}
