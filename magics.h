#ifndef MAGICS_H
#define MAGICS_H

#include "types.h"

// Macroses ////////////////////

#define RATT(sq, occ)  ( rAtt[sq][(int)((((occ) & rMagic[sq].mask) * rMagic[sq].magic) >> rMagic[sq].shift)] )
#define BATT(sq, occ)  ( bAtt[sq][(int)((((occ) & bMagic[sq].mask) * bMagic[sq].magic) >> bMagic[sq].shift)] )
#define QATT(sq, occ)  ( RATT(sq, occ) | BATT(sq, occ) )

// Classes /////////////////////

struct Magic
{
	U64 mask;
	U64 magic;
	int shift;
};

// Prototypes //////////////////

extern int transform(U64 b, U64 magic, int bits);
extern int initMagics();

// Variables ///////////////////

extern U64 bAtt[64][512];
extern U64 rAtt[64][4096];
extern Magic bMagic[64];
extern Magic rMagic[64];

#endif // MAGICS_H