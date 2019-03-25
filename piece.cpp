#include "piece.h"
#include "consts.h"
#include "magics.h"
#include "util.h"

PieceInfo pieces[PIECE_N];
U64 between[SQUARE_N][SQUARE_N];
U64 forward_one[COLOR_N][SQUARE_N];

void init_piece_from_array(int type, int moves[][2], int cnt, bool slider = false)
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			int sq = SQ(i,j);

			for (int k = 0; k < cnt; k++)
			{
				int x = i, y = j;
				do
				{
					x += moves[k][0];
					y += moves[k][1];

					if (x < 0 || x > 7 || y < 0 || y > 7) break;

					pieces[type].att[sq] |= BIT << SQ(x,y);
				}
				while (slider);
			}
		}
	}
}


void init_pieces()
{
	// White pawns
	int wp_offset[][2] =
	{
		{-1, 1}, {1, 1}
	};

	// Black pawns
	int bp_offset[][2] =
	{
		{-1,-1}, {1,-1}
	};

	// Knights
	int n_offset[][2] =
	{
		{1, 2}, {1,-2}, {-1, 2}, {-1,-2},
		{2, 1}, {2,-1}, {-2, 1}, {-2,-1}
	};

	// Kings
	int k_offset[][2] =
	{
		{-1, 1}, {0, 1}, {1, 1},
		{-1, 0},         {1, 0},
		{-1,-1}, {0,-1}, {1,-1}
	};

	int diag_offset[][2] =
	{
		{-1,-1}, {-1, 1}, {1,-1}, {1, 1}
	};

	int rook_offset[][2] =
	{
		{-1, 0}, {0, 1}, {1, 0}, {0,-1}
	};

	// Clear all //////////////////////////////////////////

	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 64; j++)
			pieces[i].att[j] = EMPTY;

	// Pawns //////////////////////////////////////////////

	init_piece_from_array(WP, wp_offset, 2);
	init_piece_from_array(BP, bp_offset, 2);

	// Knights & kings ////////////////////////////////////

	init_piece_from_array(WN, n_offset, 8);
	init_piece_from_array(BN, n_offset, 8);

	init_piece_from_array(WK, k_offset, 8);
	init_piece_from_array(BK, k_offset, 8);

	// Diagonal pieces ////////////////////////////////////

	init_piece_from_array(WB, diag_offset, 4, true);
	init_piece_from_array(BB, diag_offset, 4, true);

	init_piece_from_array(WQ, diag_offset, 4, true);
	init_piece_from_array(BQ, diag_offset, 4, true);

	// Straight pieces ////////////////////////////////////

	init_piece_from_array(WR, rook_offset, 4, true);
	init_piece_from_array(BR, rook_offset, 4, true);

	init_piece_from_array(WQ, rook_offset, 4, true);
	init_piece_from_array(BQ, rook_offset, 4, true);

	/////////////////////////////////////////////////////////////////////////

	// Pawn forward moves

	for (int i = 0; i < 64; i++)
	{
		U64 bb = BIT << i;
		pieces[WP].mov[i] = EMPTY;
		pieces[BP].mov[i] = EMPTY;

		if (Y(i) >  0) pieces[WP].mov[i] |= bb << 8;
		if (Y(i) == 1) pieces[WP].mov[i] |= bb << 16;

		if (Y(i) <  7) pieces[BP].mov[i] |= bb >> 8;
		if (Y(i) == 6) pieces[BP].mov[i] |= bb >> 16;
	}

	// Forward one (for blocks detection)
	for (int i = 0; i < 64; i++)
	{
		forward_one[0][i] = EMPTY;
		forward_one[1][i] = EMPTY;
		if (Y(i) < 7) forward_one[0][i] = (BIT << i) >> 8;
		if (Y(i) > 0) forward_one[1][i] = (BIT << i) << 8;
	}
}

void init_arrays()
{
	for (int i = 0; i < 64; i++)
	{
		for (int j = i + 1; j < 64; j++) // j > i
		{
			between[i][j] = EMPTY;
			int dx = X(j) - X(i);
			int dy = Y(j) - Y(i);

			if (ABS(dx) == ABS(dy) // Diagonal
			||  !dx || !dy)        // Orthogonal
			{
				int sx = SIGN(dx);
				int sy = SIGN(dy);
				int step = SQ( SIGN(dx), SIGN(dy) );
				for (int k = i + step; k < j; k += step)
					between[i][j] |= BIT << k;
			}

			between[j][i] = between[i][j];
		}
	}
}