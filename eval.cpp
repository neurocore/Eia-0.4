#include "eval.h"
#include "piece.h"
#include "util.h"

U64 vert[64];
U64 hori[64];
U64 isolator[64];
U64 onward[2][64];
U64 attspan[2][64];
U64 attrear[2][64];
U64 connects[2][64];
int kingzone[64][64]; // [king][field] -> index

const int material[PIECE_N] = {100, 100, 300, 300, 300, 300, 500, 500, 900, 900, 0, 0};

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