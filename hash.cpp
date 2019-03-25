#include "hash.h"
#include "util.h"

U64 hashKey[12][64];
U64 hashCastle[16];
U64 hashEp[64];
U64 hashWtm[2];

void init_hash()
{
	for (int i = 0; i < 12; i++)
		for (int j = 0; j < 64; j++)
			hashKey[i][j] = rand64();

	for (int i = 0; i < 16; i++)
		hashCastle[i] = i ? rand64() : EMPTY;

	for (int i = 0; i < 64; i++)
		hashEp[i] = (Y(i) == 2 || Y(i) == 5) ? rand64() : EMPTY;

	hashWtm[0] = rand64();
	hashWtm[1] = EMPTY;
}
