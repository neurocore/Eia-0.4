#include <iomanip>
#include <cmath>
#include "eval.h"
#include "piece.h"
#include "util.h"
#include "board.h"
#include "magics.h"

U64 vert[64];
U64 hori[64];
U64 isolator[64];
U64 onward[2][64];
U64 attspan[2][64];
U64 attrear[2][64];
U64 connects[2][64];
int kingzone[64][64]; // [king][field] -> index

const U64 matkey[12] = {MKEY_BP, MKEY_WP, MKEY_BN, MKEY_WN, MKEY_BB, MKEY_WB, MKEY_BR, MKEY_WR, MKEY_BQ, MKEY_WQ};
const int material[PIECE_N] = {100, 100, 300, 300, 300, 300, 500, 500, 900, 900, 0, 0};

int eval()
{
    int mat = MAT_WHITE->val - MAT_BLACK->val;
    int phase = MATERIAL(B->wtm ^ 1)->phase;
	int val = mat + B->state->pst.tapered(phase) + E->term[Tempo];

    val += eval_n(WHITE) - eval_n(BLACK);
    val += eval_b(WHITE) - eval_b(BLACK);
    val += eval_r(WHITE) - eval_r(BLACK);
    val += eval_q(WHITE) - eval_q(BLACK);

    return B->wtm ? val : -val;
}

int eval_n(int col)
{
    int val = 0;
    for (U64 bb = B->piece[BN + col]; bb; RLSB(bb))
    {
        int sq = BITSCAN(bb);
        U64 att = pieces[BN + col].att[sq] & ~B->occ[col];
        val += E->mob[KNIGHT][POPCNT(att)];
    }
    return val;
}

int eval_b(int col)
{
    U64 o = B->occ[0] | B->occ[1];
    int val = 0;
    for (U64 bb = B->piece[BB + col]; bb; RLSB(bb))
    {
        int sq = BITSCAN(bb);
        U64 att = BATT(sq, o) & ~B->occ[col];
        val += E->mob[BISHOP][POPCNT(att)];
    }
    return val;
}

int eval_r(int col)
{
    U64 o = B->occ[0] | B->occ[1];
    int val = 0;
    for (U64 bb = B->piece[BR + col]; bb; RLSB(bb))
    {
        int sq = BITSCAN(bb);
        U64 att = RATT(sq, o) & ~B->occ[col];
        val += E->mob[ROOK][POPCNT(att)];
    }
    return val;
}

int eval_q(int col)
{
    U64 o = B->occ[0] | B->occ[1];
    int val = 0;
    for (U64 bb = B->piece[BQ + col]; bb; RLSB(bb))
    {
        int sq = BITSCAN(bb);
        U64 att = QATT(sq, o) & ~B->occ[col];
        val += E->mob[QUEEN][POPCNT(att)];
    }
    return val;
}

void init_eval()
{
	U64 verts[8] = { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };
	U64 horis[8] = { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };

	for (int i = 0; i < 64; i++)
	{
		vert[i] = verts[X(i)];
		hori[i] = horis[Y(i)];
	}

	for (int i = 0; i < 64; i++)
	{
		isolator[i] = EMPTY;
		if (X(i) > 0) isolator[i] |= vert[i - 1];
		if (X(i) < 7) isolator[i] |= vert[i + 1];

		onward[0][i] = (BIT << i) >> 8;
		onward[1][i] = (BIT << i) << 8;
		for (int j = 0; j < 8; j++)
		{
			onward[0][i] |= onward[0][i] >> 8;
			onward[1][i] |= onward[1][i] << 8;
		}

		attspan[0][i] = pieces[BP].att[i];
		attspan[1][i] = pieces[WP].att[i];
		for (int j = 0; j < 8; j++)
		{
			attspan[0][i] |= attspan[0][i] >> 8;
			attspan[1][i] |= attspan[1][i] << 8;
		}

		attrear[0][i] = attspan[0][i] ^ isolator[i];
		attrear[1][i] = attspan[1][i] ^ isolator[i];

		connects[0][i]  = pieces[WP].att[i];
		connects[0][i] |= connects[0][i] >> 8;

		connects[1][i]  = pieces[BP].att[i];
		connects[1][i] |= connects[1][i] << 8;

		int cnt = 0;
		for (U64 bb = pieces[WK].att[i]; bb; bb = RLSB(bb), cnt++)
		{
			int j = BITSCAN(bb);
			kingzone[i][j] = cnt;
		}

		//CON(i << "\n");
		//print64(isolator[i]);
		//CON("__________________\n\n");
	}
}

/////////////////////////////////////////////////////////////////////////////////////

#undef TERM
#define TERM(x,def) term[x] = def;

Eval::Eval()
{
	for (int i = 0; i < TermsCnt; i++)
		term[i] = 0;

	TERMS;
	init();
}

#undef TERM
#define TERM(x,def) CON(left << setw(14) << #x << " " << setw(4) << term[x] << "\n");

void Eval::print()
{
	TERMS;
	CON("\n");
}

#undef TERM
#define TERM(x,def) ss << #x << ":" << term[x] << " ";

string Eval::get()
{
	stringstream ss;
	TERMS;
	return ss.str();
}

#undef TERM
#define TERM(x,def) {                                           \
	size_t found = str.find(#x, 0);                             \
	if (found != string::npos)                                  \
	{                                                           \
		size_t start = str.find(":", found);                    \
		size_t end = str.find(" ", found);                      \
		term[x] = stoi(str.substr(start + 1, end - start - 1)); \
	}}

void Eval::set(string str)
{
	TERMS;
	init();
}

#undef TERM

void Eval::set(Eval * EV)
{
	for (int i = 0; i < TermsCnt; i++)
		term[i] = EV->term[i];
    init();
}

void Eval::init()
{
    const int PFile[8] = {-3, -1, +0, +1, +1, +0, -1, -3};

    const int NLine[8] = {-4, -2, +0, +1, +1, +0, -2, -4};
    const int NRank[8] = {-2, -1, +0, +1, +2, +3, +2, +1};

    const int BLine[8] = {-3, -1, +0, +1, +1, +0, -1, -3};

    const int RFile[8] = {-2, -1, +0, +1, +1, +0, -1, -2};

    const int QLine[8] = {-3, -1, +0, +1, +1, +0, -1, -3};

    const int KLine[8] = {-3, -1, +0, +1, +1, +0, -1, -3};
    const int KFile[8] = {+3, +4, +2, +0, +0, +2, +4, +3};
    const int KRank[8] = {+1, +0, -2, -3, -4, -5, -6, -7};

    mat[NOP] = 0;
    mat[WP] = 100;
	mat[WN] = term[MatKnight];
	mat[WB] = term[MatBishop];
	mat[WR] = term[MatRook];
	mat[WQ] = term[MatQueen];
	mat[WK] = 20000;

    for (int i = 0; i < PIECE_N; i += 2)
		mat[i] = -mat[i + 1];

    for (int i = 0; i < PIECE_N; i++)
        for (int j = 0; j < 64; j++)
            pst[i][j].clear();

    // Pawns //////////////////////////////////////////////

	int p = WP; 

	// file
	for (int sq = 0; sq < 64; sq++)
	{
		pst[p][sq].op += PFile[X(sq)] * term[PawnFile];
	}

	// center control
	pst[p][D3].op += 10;
	pst[p][E3].op += 10;

	pst[p][D4].op += 20;
	pst[p][E4].op += 20;

	pst[p][D5].op += 10;
	pst[p][E5].op += 10;

	// Knights ////////////////////////////////////////////

	p = WN;

	// center
	for (int sq = 0; sq < 64; sq++)
	{
		pst[p][sq].op += NLine[X(sq)] * term[KnightCenterOp];
		pst[p][sq].op += NLine[Y(sq)] * term[KnightCenterOp];
		pst[p][sq].eg += NLine[X(sq)] * term[KnightCenterEg];
		pst[p][sq].eg += NLine[Y(sq)] * term[KnightCenterEg];
	}

	// rank
	for (int sq = 0; sq < 64; sq++)
	{
		pst[p][sq].op += NRank[Y(sq)] * term[KnightRank];
	}

	// back rank
	for (int sq = A1; sq <= H1; sq++)
	{
		pst[p][sq].op -= term[KnightBackRank];
	}

	// "trapped"
	pst[p][A8].op -= term[KnightTrapped];
	pst[p][H8].op -= term[KnightTrapped];

	// Bishops ////////////////////////////////////////////

	p = WP;

	// center
	for (int sq = 0; sq < 64; sq++)
	{
		pst[p][sq].op += BLine[Y(sq)] * term[BishopCenterOp];
		pst[p][sq].op += BLine[X(sq)] * term[BishopCenterOp];
		pst[p][sq].eg += BLine[Y(sq)] * term[BishopCenterEg];
		pst[p][sq].eg += BLine[X(sq)] * term[BishopCenterEg];
	}

	// back rank
	for (int sq = A1; sq <= H1; sq++)
	{
		pst[p][sq].op -= term[BishopBackRank];
	}

	// main diagonals
	for (int i = 0; i < 8; i++)
	{
		int sq = SQ(i,i);
		pst[p][sq].op      += term[BishopDiagonal];
		pst[p][OPP(sq)].op += term[BishopDiagonal];
	}

	// Rooks //////////////////////////////////////////////

	p = WR;

	// file
	for (int sq = 0; sq < 64; sq++)
	{
		pst[p][sq].op += RFile[X(sq)] * term[RookFileOp];
	}

	// Queens /////////////////////////////////////////////

	p = WQ;

	// center
	for (int sq = 0; sq < 64; sq++)
	{
		pst[p][sq].op += QLine[X(sq)] * term[QueenCenterOp];
		pst[p][sq].op += QLine[Y(sq)] * term[QueenCenterOp];
		pst[p][sq].eg += QLine[X(sq)] * term[QueenCenterEg];
		pst[p][sq].eg += QLine[Y(sq)] * term[QueenCenterEg];
	}

	// back rank
	for (int sq = A1; sq <= H1; sq++)
	{
		pst[p][sq].op -= term[QueenBackRank];
	}

	// Kings //////////////////////////////////////////////

	p = WK;

	for (int sq = 0; sq < 64; sq++)
	{
		pst[p][sq].op += KFile[X(sq)] * term[KingFile];
		pst[p][sq].op += KRank[Y(sq)] * term[KingRank];
		pst[p][sq].eg += KLine[X(sq)] * term[KingCenterEg];
		pst[p][sq].eg += KLine[Y(sq)] * term[KingCenterEg];
	}

	// Symmetrical copy for black /////////////////////////

    for (int i = 0; i < 12; i += 2)
    {
        for (int sq = 0; sq < 64; sq++)
        {
            pst[i][sq].op = -pst[i + 1][OPP(sq)].op;
            pst[i][sq].eg = -pst[i + 1][OPP(sq)].eg;
        }
    }

    // Initialization of material data ////////////////////

    for (int pi = 0; pi <= 8; pi++)
    {
        for (int ni = 0; ni <= 10; ni++)
        {
            for (int bi = 0; bi <= 10; bi++)
            {
                for (int ri = 0; ri <= 10; ri++)
                {
                    for (int qi = 0; qi <= 9; qi++)
                    {
                        int mkey = pi * MKEY_BP + ni * MKEY_BN + bi * MKEY_BB + ri * MKEY_BR + qi * MKEY_BQ;
                        Material * m = mat_table + mkey;

                        m->phase = (ni + bi) * PLight + ri * PRook + qi * PQueen;
                        m->val = pi * mat[WP] + ni * mat[WN] + bi * mat[WB] + ri * mat[WR] + qi * mat[WQ];
                    }
                }
            }
        }
    }

    // Mobility (logarithm -> entrophy) ////////////////////

    const double k = exp(1) - 1;
    for (int c = 0; c < 28; c++)
    {
        mob[PAWN   ][c] = 0;
        mob[KNIGHT ][c] = term[NMob] * log(1 + k * c /  8.0);
        mob[BISHOP ][c] = term[BMob] * log(1 + k * c / 13.0);
        mob[ROOK   ][c] = term[RMob] * log(1 + k * c / 14.0);
        mob[QUEEN  ][c] = term[QMob] * log(1 + k * c / 27.0);
        mob[KING   ][c] = 0;
    }
}