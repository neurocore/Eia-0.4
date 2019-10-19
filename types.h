#ifndef TYPES_H
#define TYPES_H

#include <climits>
#include <cassert>
#include <iostream>
#include "options.h"

typedef unsigned char uchar;
typedef unsigned long long U64;
typedef unsigned int U32;

using namespace std;

#define L(x)       ( x##ULL )

#define LSB(x)     ( (x) & (EMPTY - (x)) )
#define MSB(x)     ( (x) |= (x) >> 1,    \
                     (x) |= (x) >> 2,    \
                     (x) |= (x) >> 4,    \
                     (x) |= (x) >> 8,    \
                     (x) |= (x) >> 16,   \
                     ((x) >> 1) + 1      )

#define RLSB(x)    ( (x) = (x) & ((x) - 1) )

#define SQ(x,y)    ( ((y) << 3) + (x) )
#define SQR(x,y)   ( ((y) * 8) + (x) )
#define X(sq)      ( (sq) & 0x07 )
#define Y(sq)      ( (sq) >> 3 )
#define OPP(sq)    ( (sq) ^ 070 )

#define X88(sq)    ( (sq) & 0x07 )
#define Y88(sq)    ( (sq) >> 4 )
#define SQ88(x,y)  ( ((y) << 4) + (x) )

#define MAX(x,y)   ( (x) > (y) ? (x) : (y) )
#define MIN(x,y)   ( (x) < (y) ? (x) : (y) )
#define ABS(x)     ( (x) > 0 ? (x) : -(x) )
#define SIGN(x)    ( (x) == 0 ? 0 : ( (x) > 0 ? 1 : -1 ) )
#define NOTZERO(x) ( ((x) == 0) ? 1 : (x) )

#define COMPARE(x, y, lt, eq, gt) ( (x) < (y) ? (lt) : ((x) > (y) ? (gt) : (eq)) )

#define SQ_CON(sq)  FILES[X(sq)] << RANKS[Y(sq)]
#define SQ_OUT(sq)  FILOW[X(sq)] << RANKS[Y(sq)]

#define IS_PAWN(p) ( (p) < BN )
#define TYPE(p)    ( (p) >> 1 )
#define COL(p)     ( (p) & 1 )

#define FILE(x)    ( L(0x0101010101010101) << (x) )
#define RANK(x)    ( L(0x00000000000000FF) << ((x) * 8) )

#ifdef _DEBUG
#define ASSERT(x)  { if (!(x)) { B->print(); }; assert(x); }
#else
#define ASSERT(x)  { }
#endif

#ifdef _DEBUG
#define ASSERT_MOVE(x,m)  { if (!(x)) { B->print(); CON(m << "\n") }; assert(x); }
#else
#define ASSERT_MOVE(x,m)  { }
#endif

#ifdef LOGGING
#define LOG(s)     { S->flog << s << flush; }
#else              
#define LOG(s)     {}
#endif             

#define OUT(s)     { cout << s; LOG(s); }
#define INP(s)     { LOG(">> " << s << endl); }
#define FLUSH      { cout << flush; }

#ifdef PRODUCTION  
#define CON(s)     {}
#else              
#define CON(s)     { if (console) cout << s; }
#endif

enum Color     { BLACK, WHITE, COLOR_N };
enum Piece     { BP, WP, BN, WN, BB, WB, BR, WR, BQ, WQ, BK, WK, PIECE_N, NOP = 13 };
enum PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_N };

// Macro for automatic enums creation //////////////////

#define SQUARES                                  \
_(A1) _(B1) _(C1) _(D1) _(E1) _(F1) _(G1) _(H1)  \
_(A2) _(B2) _(C2) _(D2) _(E2) _(F2) _(G2) _(H2)  \
_(A3) _(B3) _(C3) _(D3) _(E3) _(F3) _(G3) _(H3)  \
_(A4) _(B4) _(C4) _(D4) _(E4) _(F4) _(G4) _(H4)  \
_(A5) _(B5) _(C5) _(D5) _(E5) _(F5) _(G5) _(H5)  \
_(A6) _(B6) _(C6) _(D6) _(E6) _(F6) _(G6) _(H6)  \
_(A7) _(B7) _(C7) _(D7) _(E7) _(F7) _(G7) _(H7)  \
_(A8) _(B8) _(C8) _(D8) _(E8) _(F8) _(G8) _(H8)

#define EMPTY     L(0x0000000000000000)
#define FULL      L(0xFFFFFFFFFFFFFFFF)
#define BIT       L(0x0000000000000001)
#define LIGHT     L(0x55AA55AA55AA55AA)
#define DARK       (~LIGHT)

#define FILE_A    L(0x0101010101010101)
#define FILE_B    L(0x0202020202020202)
#define FILE_C    L(0x0404040404040404)
#define FILE_D    L(0x0808080808080808)
#define FILE_E    L(0x1010101010101010)
#define FILE_F    L(0x2020202020202020)
#define FILE_G    L(0x4040404040404040)
#define FILE_H    L(0x8080808080808080)
                  
#define RANK_1    L(0x00000000000000FF)
#define RANK_2    L(0x000000000000FF00)
#define RANK_3    L(0x0000000000FF0000)
#define RANK_4    L(0x00000000FF000000)
#define RANK_5    L(0x000000FF00000000)
#define RANK_6    L(0x0000FF0000000000)
#define RANK_7    L(0x00FF000000000000)
#define RANK_8    L(0xFF00000000000000)

// Directions and shifts ///////////////////////////////

#define SHIFT__U(b) (  (b) << 8 )
#define SHIFT__D(b) (  (b) >> 8 )
#define SHIFT__R(b) ( ((b) & ~FILE_H) << 1 )
#define SHIFT__L(b) ( ((b) & ~FILE_A) >> 1 )
#define SHIFT_UR(b) ( ((b) & ~FILE_A) << 7 )
#define SHIFT_UL(b) ( ((b) & ~FILE_H) << 9 )
#define SHIFT_DR(b) ( ((b) & ~FILE_A) >> 9 )
#define SHIFT_DL(b) ( ((b) & ~FILE_H) >> 7 )

enum Dir { DIR__U, DIR__D, DIR__L, DIR__R,
           DIR_UR, DIR_UL, DIR_DR, DIR_DL };

static U64 shift(U64 bb, Dir dir)
{
    switch (dir)
    {
        case DIR__U: return SHIFT__U(bb);
        case DIR__D: return SHIFT__D(bb);
        case DIR__L: return SHIFT__L(bb);
        case DIR__R: return SHIFT__R(bb);
        case DIR_UR: return SHIFT_UR(bb);
        case DIR_UL: return SHIFT_UL(bb);
        case DIR_DR: return SHIFT_DR(bb);
        case DIR_DL: return SHIFT_DL(bb);
    }
}

// Castlings ///////////////////////////////////////////

#define C_BK      (1 << 0)
#define C_BQ      (1 << 1)
#define C_WK      (1 << 2)
#define C_WQ      (1 << 3)
#define C_ALL     (C_BK | C_BQ | C_WK | C_WQ)

// Squares /////////////////////////////////////////////

#define _(x) x,
enum Square { SQUARES SQUARE_N };
#undef _(x)

#define _(x) constexpr U64 s##x = (BIT << x);
SQUARES
#undef _(x)

// Moves ///////////////////////////////////////////////

enum Flags
{
	F_QUIET,     // PC12 (Promotion, Capture, ...)
	F_PAWN2,
	F_KCASTLE,   // 0010
	F_QCASTLE,   // 0011
	F_CAP,       // 0100
	F_EP,	     // 0101
	F_NPROM = 8, // 1000
	F_BPROM,     // 1001
	F_RPROM,     // 1010
	F_QPROM,     // 1011
	F_NCAPPROM,  // 1100
	F_BCAPPROM,  // 1101
	F_RCAPPROM,  // 1110
	F_QCAPPROM   // 1111
};

// Move type - 16 bits. It's compact enough to be stored in hash table and to extract values from it

#define MOVE(from, to, flags)   ( Move((from) | ((to) << 6) | ((flags) << 12)) )
#define FLAGS(move)             ( (move) >> 12 )
#define FROM(move)              ( (move) & 077 )
#define TO(move)                (((move) >> 6) & 077 )

#define MV_OUT(m)               SQ_OUT(FROM(m)) << SQ_OUT(TO(m))

enum Move
{
    MOVE_NONE = MOVE(0, 0, 0), // a1a1
    MOVE_NULL = MOVE(1, 1, 0)  // b1b1
};

#define IS_EMPTY(move)          ( (move) == MOVE_NONE )
#define IS_VALID(move)          ( (move) != MOVE_NONE && (move) != MOVE_NULL )

#define IS_CAP(flags)           ( (flags) & F_CAP )
#define IS_PROM(flags)          ( (flags) & F_NPROM )
#define IS_CAP_OR_PROM(flags)   ( (flags) & F_NCAPPROM )
#define IS_EP(flags)            ( (flags) == F_EP )
#define IS_PAWN2(flags)         ( (flags) == F_PAWN2 )
#define IS_CASTLE(flags)        ( (flags) == F_KCASTLE || (flags) == F_QCASTLE )
#define N_PROM(flags)           ( (flags) & 3 )

// Eval ////////////////////////////////////////////////

enum Phase { OP, EG };

#define PLight    1
#define PRook     2
#define PQueen    4
#define PTotal    (PLight * 4 + PRook * 2 + PQueen)
#define PEndgame  7

#define TAPERED(phase, op, eg) ((((op) * (phase)) + ((eg) * (PTotal - phase))) / PTotal)

// Material key ////////////////////////////////////////

/**
 *  Holds individual counts of pieces on board, but
 *   designed to get precalculated material entry
 *   that consists of material score, phase and
 *   endgame recognition stuff just for one color.
 *  So material key is simple to update and little
 *   harder to get values from it.
 */

constexpr const U64 MKEY_BP = 1;
constexpr const U64 MKEY_BN = MKEY_BP * 9;
constexpr const U64 MKEY_BB = MKEY_BN * 11;
constexpr const U64 MKEY_BR = MKEY_BB * 11;
constexpr const U64 MKEY_BQ = MKEY_BR * 11;
constexpr const U64 MKEYS   = MKEY_BQ * 10;

constexpr const U64 MKEY_WP = BIT << 32;
constexpr const U64 MKEY_WN = MKEY_WP * 9;
constexpr const U64 MKEY_WB = MKEY_WN * 11;
constexpr const U64 MKEY_WR = MKEY_WB * 11;
constexpr const U64 MKEY_WQ = MKEY_WR * 11;
constexpr const U64 MKEY_SZ = MKEY_WQ * 10;

static_assert(MKEYS < INT_MAX, "Material keys are not compact enough");

#define MAT_BLACK_(mkey)      (E->mat_table + ((mkey) & 0xFFFFFFFF))
#define MAT_WHITE_(mkey)      (E->mat_table + ((mkey) >> 32))
#define MATERIAL_(wtm, mkey)  (wtm ? MAT_WHITE_(mkey) : MAT_BLACK_(mkey))

#define MAT_BLACK             MAT_BLACK_(B->state->mkey)
#define MAT_WHITE             MAT_WHITE_(B->state->mkey)
#define MATERIAL(wtm)         MATERIAL_(wtm, B->state->mkey)

// Search //////////////////////////////////////////////

#define PLY          ( (int)(B->state - B->undo) )
#define ROOT_WINDOW  100

// Other ///////////////////////////////////////////////

#define FILES     "ABCDEFGH"
#define FILOW     "abcdefgh"
#define RANKS     "12345678"
                  
#define MB        (1 << 20)

extern bool console;

    
#endif // TYPES_H
