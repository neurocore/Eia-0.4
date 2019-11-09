#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "consts.h"

#define TERM(x,def)            x,
#define TERMS                  \
	TERM(MatKnight,      320)  \
	TERM(MatBishop,      330)  \
	TERM(MatRook,        500)  \
	TERM(MatQueen,       900)  \
	TERM(PawnFile,         5)  \
	TERM(KnightCenterOp,   5)  \
	TERM(KnightCenterEg,   5)  \
	TERM(KnightRank,       5)  \
	TERM(KnightBackRank,   0)  \
	TERM(KnightTrapped,  100)  \
	TERM(BishopCenterOp,   2)  \
	TERM(BishopCenterEg,   3)  \
	TERM(BishopBackRank,  10)  \
	TERM(BishopDiagonal,   4)  \
	TERM(RookFileOp,       3)  \
	TERM(QueenCenterOp,    0)  \
	TERM(QueenCenterEg,    4)  \
	TERM(QueenBackRank,    5)  \
	TERM(KingFile,        10)  \
	TERM(KingRank,        10)  \
	TERM(KingCenterEg,    22)  \
	TERM(Doubled,         14)  \
	TERM(Isolated,        10)  \
	TERM(Hole,            10)  \
	TERM(NMob,            20)  \
	TERM(BMob,            10)  \
	TERM(RMob,             8)  \
	TERM(QMob,             6)  \
	TERM(BishopPair,      13)  \
	TERM(BadBishop,       38)  \
	TERM(KnightOutpost,   10)  \
	TERM(RookSemi,        10)  \
	TERM(RookOpen,        20)  \
	TERM(Rook7thOp,       20)  \
	TERM(Rook7thEg,       12)  \
	TERM(BadRook,         20)  \
	TERM(PawnFork,        10)  \
	TERM(KnightFork,      20)  \
	TERM(BishopFork,      13)  \
	TERM(HolePress,        8)  \
	TERM(ContactCheckR,  100)  \
	TERM(ContactCheckQ,  180)  \
	TERM(Shield1,         10)  \
	TERM(Shield2,          3)  \
	TERM(Shield3,        -15)  \
	TERM(Candidate,      100)  \
	TERM(CandidateK,      32)  \
	TERM(Passer,         100)  \
	TERM(PasserK,         32)  \
	TERM(PasserSupport,  150)  \
	TERM(PasserSupportK,  32)  \
	TERM(PinMul,           5)  \
	TERM(XrayMul,          3)  \
	TERM(Tempo,           15)   

enum Terms
{
	TERMS
	TermsCnt
};

struct Val // TODO: is it real to make it U64 (two signed ints)?
{
    int op = 0, eg = 0;

    Val(int op = 0, int eg = 0) : op(op), eg(eg)
    {}

    void clear()
    {
        op = eg = 0;
    }

    inline int tapered(int phase)
    {
        return ((op * phase) + (eg * (PTotal - phase))) / PTotal;
    }

    Val operator + (const Val & v) { return Val(op + v.op, eg + v.eg); }
    Val operator - (const Val & v) { return Val(op - v.op, eg - v.eg); }

    Val & operator  = (const Val & v) { op  = v.op; eg  = v.eg; return *this; }
    Val & operator += (const Val & v) { op += v.op; eg += v.eg; return *this; }
    Val & operator -= (const Val & v) { op -= v.op; eg -= v.eg; return *this; }
};

struct Material
{
    short val;
    uchar phase;
    uchar __1;
};

struct Eval
{
    int term[TermsCnt];

    Val pst[PIECE_N][SQUARE_N];
    int mat[PIECE_N + 1];
    int mob[PIECE_TYPE_N][30];
    Material mat_table[MKEYS];

    Eval();

    void init();
    void print();

	string get();
	void set(string str);
	void set(Eval * EV);
};

extern Eval * E;

extern U64 vert[64];
extern U64 hori[64];
extern U64 isolator[64];
extern U64 onward[2][64];
extern U64 attspan[2][64];
extern U64 attrear[2][64];
extern U64 connects[2][64];
extern int kingzone[64][64]; // [king][field] -> index
extern const U64 matkey[12];

extern void init_eval();
extern int  eval();
extern int  eval_p(int col);
extern int  eval_n(int col);
extern int  eval_b(int col);
extern int  eval_r(int col);
extern int  eval_q(int col);


#endif // EVAL_H
