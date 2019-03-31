#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "consts.h"

struct Eval
{
    const int material[PIECE_N] = {100, 100, 300, 300, 300, 300, 500, 500, 900, 900, 10000, 10000};
    int pst[PIECE_N][SQUARE_N];
    int mat[PIECE_N];

    Eval()
    {
        for (int i = 0; i < PIECE_N; i++)
        {
            mat[i] = i & 1 ? material[i] : -material[i];
            for (int j = 0; j < 64; j++)
                pst[i][j] = 0;
        }
    }
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
extern const int material[PIECE_N];

extern void init_eval();

#endif // EVAL_H
