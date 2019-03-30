#ifndef PIECE_H
#define PIECE_H

#include "types.h"
#include "consts.h"

// Classes /////////////////////

struct PieceInfo
{
	U64 att[SQUARE_N]; // Attack tables for every square piece in
	U64 mov[SQUARE_N]; // Move tables for pawns
};

// Variables ///////////////////

extern int x88_sq[256];
extern int sq_x88[256];
extern PieceInfo pieces[PIECE_N];
extern int dir[SQUARE_N][SQUARE_N];
extern U64 between[SQUARE_N][SQUARE_N];
extern U64 forward_one[COLOR_N][SQUARE_N];

// Prototypes //////////////////

extern void init_pieces();
extern void init_arrays();


#endif // PIECE_H
