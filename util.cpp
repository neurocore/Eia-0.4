#include <cstdlib>
#include "util.h"

unsigned long btscn_indx;
unsigned int lut[65536];

void init_lut()
{
    for (int i = 0; i < 65536; i++)
    {
        int cnt = 0;
        for (int j = i; j; RLSB(j)) cnt++;
        lut[i] = cnt;
    }
}

U64 rand64()
{
    U64 r;
    r  = rand() & 0xFF;
    r <<= 8;
    r |= rand() & 0xFF;
    r <<= 8;
    r |= rand() & 0xFF;
    r <<= 8;
    r |= rand() & 0xFF;
    r <<= 8;
    r |= rand() & 0xFF;
    r <<= 8;
    r |= rand() & 0xFF;
    r <<= 8;
    r |= rand() & 0xFF;
    r <<= 8;
    r |= rand() & 0xFF;
    return r;
}

/////////////////////////////////////////////////////

void print64(U64 bb)
{
    for (int i = 7; i >= 0; i--)
    {
        CON( RANKS[i] << "|" );
        for (int j = 0; j < 8; j++)
        {
            if (bb & (BIT << SQ(j, i))) CON( "x" )
            else                        CON( "." );
        }
        CON( "\n" );
    }
    CON( " +--------\n  " << FILES << "\n\n" );
}
