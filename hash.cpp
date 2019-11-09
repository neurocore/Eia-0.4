#include <random>
#include "hash.h"
#include "util.h"
#include "board.h"
#include "search.h"

U64 hash_key[12][64];
U64 hash_castle[16];
U64 hash_ep[64];
U64 hash_wtm;

using namespace std;

void init_hash()
{
    mt19937_64 gen(0xCEED);
    uniform_int_distribution<U64> dist;

	for (int i = 0; i < 12; i++)
		for (int j = 0; j < 64; j++)
			hash_key[i][j] = dist(gen);

	for (int i = 0; i < 16; i++)
		hash_castle[i] = i ? dist(gen) : EMPTY;

	for (int i = 0; i < 64; i++)
		hash_ep[i] = dist(gen);

	hash_wtm = dist(gen);
}

Hash::Hash(int sizeMb)
{
	init(sizeMb);
}

Hash::~Hash()
{
	_delete();
}

void Hash::_delete()
{
	if (table)
	{
		delete[] table;
		table = 0;
	}
}

void Hash::init(int sizeMb)
{
	size = sizeMb * MB / sizeof(HashEntry);
	size = MSB(size);
	table = new HashEntry[size];
	clear();
}

void Hash::clear()
{
	for (HashEntry * the = table; the - table < size; the++)
	{
		the->key = EMPTY;
		the->move = Move();
		the->type = 0;
		the->depth = 0;
		the->val = 0;
	}
}

HashEntry * Hash::get(U64 key)
{
	HashEntry * the = table + (key & (size - 1));
    if (the->key == key)
    {
        S->hash_read++;
        return the;
    }
	return NULL;
}

HashEntry * Hash::get(U64 key, int & alpha, int & beta, int depth)
{
	HashEntry * the = table + (key & (size - 1));
	if (the->key == key)
	{
		S->hash_read++;

        if (the->depth >= depth)
        {
            int val = the->val;
            if      (val >  MATE && val <=  INF) val -= PLY;
            else if (val < -MATE && val >= -INF) val += PLY;

            // Exact score
            if (the->type == Hash_Exact) alpha = beta = val;
            else if (the->type == Hash_Alpha && val <= alpha) beta = alpha;
            else if (the->type == Hash_Beta  && val >= beta) alpha = beta;
        }
		return the;
	}
	return NULL;
}

void Hash::set(U64 key, Move move, int depth, int val, int type)
{
	S->hash_write++;
    short age = S->movecnt;
	HashEntry * the = table + (key & (size - 1));
	
	//if (/*the->age == age && */the->depth > depth) return;

    if      (val >  MATE && val <=  INF) val += PLY;
	else if (val < -MATE && val >= -INF) val -= PLY;

	the->key = key;
	the->move = move;
	the->type = type;
	the->depth = depth;
	the->val = val;
    the->age = age;
}
