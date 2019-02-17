#ifndef UTIL_H
#define UTIL_H

#include "types.h"

// Consts ////////////////////////

const int btscn64[64] =
{
     0,  1, 48,  2, 57, 49, 28,  3,
    61, 58, 50, 42, 38, 29, 17,  4,
    62, 55, 59, 36, 53, 51, 43, 22,
    45, 39, 33, 30, 24, 18, 12,  5,
    63, 47, 56, 27, 60, 41, 37, 16,
    54, 35, 52, 21, 44, 32, 23, 11,
    46, 26, 40, 15, 34, 20, 31, 10,
    25, 14, 19,  9, 13,  8,  7,  6
};

// Macroses //////////////////////

#define DEBRUIJN64     L(0x03f79d71b4cb0a89)
#define BITSCAN(x)     ( btscn64[(LSB(x) * DEBRUIJN64) >> 58] )
#define POPCNT(x)      ( lut[ (x) >> 48          ] + lut[((x) >> 32) & 0xFFFF] \
                       + lut[((x) >> 16) & 0xFFFF] + lut[ (x)        & 0xFFFF] )

// Prototypes ////////////////////

extern U64 rand64();
extern void initLut();
extern void print64(U64);

// Variables /////////////////////

extern unsigned int lut[65536];


#endif // UTIL_H
