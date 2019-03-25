#ifndef HASH_H
#define HASH_H

#include "types.h"
#include "consts.h"
#include "moves.h"

// Consts ////////////////////////////////////

#define HASH_SIZE  8
#define PHASH_SIZE 2

enum ValueType
{
    Hash_Alpha,
    Hash_Exact,
    Hash_Beta
};

// Classes ///////////////////////////////////

struct HashEntry // 18
{
    U64 key;     // 8
    short move;  // 2
    uchar type;  // 1
    uchar depth; // 1
    short age;   // 2
    int val;     // 4
};

struct Hash
{
    int size;
    HashEntry * he;

    Hash(int sizeMb = HASH_SIZE);
    ~Hash();
    void _delete();

    void init(int sizeMb);
    void clear();
    HashEntry * get(U64 key);
    HashEntry * get(U64 key, int & alpha, int & beta, int depth);
    void set(U64 key, Move move, int depth, int val, int type);
};

struct PHashEntry
{
    U64 key;
    U64 open[2];
    U64 holes[2];
    U64 blocked[2];
    int val;
};

struct PHash
{
    int size;
    PHashEntry * pe;

    PHash(int sizeMb = PHASH_SIZE);
    ~PHash();
    void _delete();

    void init(int sizeMb);
    void clear();
    PHashEntry * get(U64 key);
    void set(U64 key, U64 * open, U64 * holes, U64 * blocked, int val);
};

// Variables /////////////////////////////////

extern U64 hashKey[12][64];
extern U64 hashCastle[16];
extern U64 hashEp[64];
extern U64 hashWtm[2];

// Prototypes ////////////////////////////////

extern void init_hash();
extern U64  calc_phash();

#endif // HASH_H
