#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <iostream>

typedef unsigned char uchar;
typedef unsigned long long U64;

using namespace std;

#define MAX_MOVES  256
#define MAX_PLY    128

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
#define X(sq)      ( (sq) & 0x07 )
#define Y(sq)      ( (sq) >> 3 )
#define OPP(sq)    ( (sq) ^ 070 )

#define MAX(x,y)   ( (x) > (y) ? (x) : (y) )
#define MIN(x,y)   ( (x) < (y) ? (x) : (y) )
#define ABS(x)     ( (x) > 0 ? (x) : -(x) )
#define SIGN(x)    ( (x) == 0 ? 0 : ( (x) > 0 ? 1 : -1 ) )
#define NOTZERO(x) ( ((x) == 0) ? 1 : (x) )

#define SQ_OUT(sq)  FILES[X(sq)] << RANKS[Y(sq)]

#define IS_PAWN(p) ( (p) < BN )
#define TYPE(p)    ( (p) >> 1 )
#define COL(p)     ( (p) & 1 )

#define FILE(x)    ( L(0x0101010101010101) << (x) )
#define RANK(x)    ( L(0x00000000000000FF) << (x * 8) )

#define SHIFT__U(b) (  (b) << 8 )
#define SHIFT__D(b) (  (b) >> 8 )
#define SHIFT__R(b) ( ((b) & ~FILE_H) << 1 )
#define SHIFT__L(b) ( ((b) & ~FILE_A) >> 1 )
#define SHIFT_UR(b) ( ((b) & LL(0x00fefefefefefefe)) << 7 )
#define SHIFT_UL(b) ( ((b) & LL(0x007f7f7f7f7f7f7f)) << 9 )
#define SHIFT_DR(b) ( ((b) & LL(0xfefefefefefefe00)) >> 9 )
#define SHIFT_DL(b) ( ((b) & LL(0x7f7f7f7f7f7f7f00)) >> 7 )

#define ASSERT(x)  { if (!(x)) { S->board->print(); }; assert(x); }

#ifdef LOGGING
#define LOG(s)     { S->flog << s << endl << flush; }
#else              
#define LOG(s)     {}
#endif             

#define OUT(s)     { cout << s; LOG(s); }
#define INP(s)     { LOG(">> " << s << "\n"); }
#define FLUSH      { cout << flush; }

#ifdef PRODUCTION  
#define CON(s)     {}
#else              
#define CON(s)     { if (console) cout << s; }
#endif

enum Move      { MOVE_NONE = SQ(0, 0), MOVE_NULL = SQ(1, 1) };
enum Color     { WHITE, BLACK, COLOR_N };
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

// Squares /////////////////////////////////////////////

#define _(x) x,
enum Square { SQUARES SQUARE_N };
#undef _(x)

#define _(x) constexpr U64 s##x = (BIT >> x);
SQUARES
#undef _(x)

// Other ///////////////////////////////////////////////

#define FILES     "ABCDEFGH"
#define FILOW     "abcdefgh"
#define RANKS     "12345678"
                  
#define MB        (1 << 20)

extern bool console;

    
#endif // TYPES_H