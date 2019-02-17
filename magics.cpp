#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "magics.h"
#include "types.h"
#include "util.h"

using namespace std;

const int RBits[64] =
{
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};
 
const int BBits[64] =
{
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

#ifdef MAGICS_RECALC
U64 RMagic[64], BMagic[64];
#else
const U64 RMagic[64] =
{
    L(0xE480004000928128), // A1
    L(0x4040081000402000), // B1
    L(0x0080100008200080), // C1
    L(0x2280058800803000), // D1
    L(0x0080040008000280), // E1
    L(0x21000A0400080300), // F1
    L(0x0400210800900204), // G1
    L(0x0280008000442100), // H1
    L(0x10148000C0062480), // A2
    L(0x4040804000802000), // B2
    L(0x0002001480420024), // C2
    L(0x24020010440A0020), // D2
    L(0x6A12002004120008), // E2
    L(0x0202000200100408), // F2
    L(0x0114001210010804), // G2
    L(0x08420000A4010042), // H2
    L(0x4100308008804000), // A3
    L(0x021080802000C001), // B3
    L(0x000082002600C011), // C3
    L(0x1002020010082440), // D3
    L(0x0400110005000800), // E3
    L(0x0044008024120080), // F3
    L(0x1001140042411018), // G3
    L(0x2001820000804904), // H3
    L(0x000040048008208E), // A4
    L(0x0010200080804000), // B4
    L(0x8040600080100582), // C4
    L(0x0200080080100081), // D4
    L(0x0086080080800400), // E4
    L(0x2801040080220080), // F4
    L(0x44020012000C0821), // G4
    L(0xA080240200148141), // H4
    L(0x12084000A2800080), // A5
    L(0x4000410202002882), // B5
    L(0x0281144101002000), // C5
    L(0x0000080080801000), // D5
    L(0x9001802402800800), // E5
    L(0x0002000882001044), // F5
    L(0x0040280304001002), // G5
    L(0x0401008142001421), // H5
    L(0x04850040820A0020), // A6
    L(0x0090004020004000), // B6
    L(0x0000402005010030), // C6
    L(0x1080900008008080), // D6
    L(0x102A02A00A0A0010), // E6
    L(0x5102002804120090), // F6
    L(0x00808801104C0012), // G6
    L(0x040110CC10A20001), // H6
    L(0x1080108840002080), // A7
    L(0x7000408210210200), // B7
    L(0x0202100084200080), // C7
    L(0x2000080180100080), // D7
    L(0x2443011008000500), // E7
    L(0x0400040002008080), // F7
    L(0x4003022110284400), // G7
    L(0x500A805104842200), // H7
    L(0x000A210448708001), // A8
    L(0x0810851100420026), // B8
    L(0x1000200440110109), // C8
    L(0x0022141001210009), // D8
    L(0x0083000450020801), // E8
    L(0x0003000804000209), // F8
    L(0x082108012210088C), // G8
    L(0x0A2004002082D302)  // H8
};

const U64 BMagic[64] =
{
    L(0x9050500118082840), // A1
    L(0x1449500100C10800), // B1
    L(0x002800A400910000), // C1
    L(0x000410C200001100), // D1
    L(0x2001104000814024), // E1
    L(0x0102021004008200), // F1
    L(0xD0012801101810B2), // G1
    L(0x340D420801011100), // H1
    L(0x0111108401080200), // A2
    L(0x205BA0080208D9C1), // B2
    L(0x00420411060A0206), // C2
    L(0x8000090401080000), // D2
    L(0x3200011041001012), // E2
    L(0x5020008820080080), // F2
    L(0x8048845110101008), // G2
    L(0x10100901C8041C02), // H2
    L(0x01101C4082A60408), // A3
    L(0x8008000408208400), // B3
    L(0x0001001001020810), // C3
    L(0x0024000804600800), // D3
    L(0x0001000811402008), // E3
    L(0x0001000201048200), // F3
    L(0x0004000209010828), // G3
    L(0x2040820200444204), // H3
    L(0x0010480084085000), // A4
    L(0x0006080011101094), // B4
    L(0x00880100580C4501), // C4
    L(0x0020080004882020), // D4
    L(0x8000840000802010), // E4
    L(0xA010012141008808), // F4
    L(0x608C048018482400), // G4
    L(0x2804848241040080), // H4
    L(0x4010583801841000), // A5
    L(0x0008380806040140), // B5
    L(0x0000404040080A00), // C5
    L(0x0800040108440100), // D5
    L(0x00004140400C0100), // E5
    L(0x0002084202050083), // F5
    L(0x1002040040051820), // G5
    L(0x800405C200004100), // H5
    L(0x3084440440382480), // A6
    L(0x0482111022800800), // B6
    L(0x0BA2005848008401), // C6
    L(0x0901854202800800), // D6
    L(0x0284840810100203), // E6
    L(0x00A0060442031040), // F6
    L(0x0403221411006400), // G6
    L(0x40500199250001C0), // H6
    L(0x0010880808040001), // A7
    L(0x02048080D0100880), // B7
    L(0x0021010088040008), // C7
    L(0x0212020C84042000), // D7
    L(0x0000401006060004), // E7
    L(0x0000240810850002), // F7
    L(0x01C0110425104000), // G7
    L(0x8084300200510600), // H7
    L(0x052200844402A001), // A8
    L(0x0400010080A42100), // B8
    L(0x1008300202031408), // C8
    L(0x0006000808218800), // D8
    L(0x5028200440050100), // E8
    L(0x0001424420044100), // F8
    L(0x0002102002228204), // G8
    L(0x40100C0298A20200)  // H8
};
#endif

U64 bAtt[64][512];
U64 rAtt[64][4096];
Magic bMagic[64];
Magic rMagic[64];

// Prototypes //////////////////

U64 rand64Few();
U64 indexTo64(int index, int bits, U64 mask);
U64 rmask(int sq);
U64 bmask(int sq);
U64 ratt(int sq, U64 block);
U64 batt(int sq, U64 block);
U64 findMagic(int sq, int m, int bishop);
void buildMagics();

////////////////////////////////

U64 rand64Few()
{
	return rand64() & rand64() & rand64();
}

U64 indexTo64(int index, int bits, U64 mask)
{
	U64 result = EMPTY;
	for (int i = 0; i < bits; i++)
	{
		U64 b = LSB(mask);
		if (index & (1 << i)) result |= b;
		mask &= ~b;
	}
	return result;
}

U64 trace(U64 bb, Dir dir, U64 mask = FULL)
{
    U64 result = EMPTY;
    for (bb = shift(bb, dir) & mask; bb; bb = shift(bb, dir) & mask)
        result |= bb;
    return result;
}

U64 tracePre(U64 bb, Dir dir)
{
    U64 result = EMPTY;
    U64 temp = EMPTY;

    for (bb = shift(bb, dir); bb; bb = shift(bb, dir))
    {
        result |= temp;
        temp = bb;
    }
    return result;
}

U64 rmask(int sq)
{
    U64 bb = BIT << sq;
	return tracePre(bb, DIR__L) | tracePre(bb, DIR__R)
         | tracePre(bb, DIR__U) | tracePre(bb, DIR__D);
}
 
U64 bmask(int sq)
{
    U64 bb = BIT << sq;
	return tracePre(bb, DIR_UR) | tracePre(bb, DIR_UL)
         | tracePre(bb, DIR_DR) | tracePre(bb, DIR_DL);
}
 
U64 ratt(int sq, U64 block)
{
	U64 result = EMPTY;
	int rk = sq/8, fl = sq%8, r, f;
	for (r = rk+1; r <= 7; r++)
	{
		result |= BIT << (fl + r*8);
		if (block & (BIT << (fl + r*8))) break;
	}
	for (r = rk-1; r >= 0; r--)
	{
		result |= BIT << (fl + r*8);
		if (block & (BIT << (fl + r*8))) break;
	}
	for (f = fl+1; f <= 7; f++)
	{
		result |= BIT << (f + rk*8);
		if (block & (BIT << (f + rk*8))) break;
	}
	for (f = fl-1; f >= 0; f--)
	{
		result |= BIT << (f + rk*8);
		if (block & (BIT << (f + rk*8))) break;
	}
	return result;
}
 
U64 batt(int sq, U64 block)
{
	U64 result = EMPTY;
	int rk = sq/8, fl = sq%8, r, f;
	for (r = rk+1, f = fl+1; r <= 7 && f <= 7; r++, f++)
	{
		result |= (BIT << (f + r*8));
		if (block & (BIT << (f + r * 8))) break;
	}
	for (r = rk+1, f = fl-1; r <= 7 && f >= 0; r++, f--)
	{
		result |= (BIT << (f + r*8));
		if (block & (BIT << (f + r * 8))) break;
	}
	for (r = rk-1, f = fl+1; r >= 0 && f <= 7; r--, f++)
	{
		result |= (BIT << (f + r*8));
		if (block & (BIT << (f + r * 8))) break;
	}
	for (r = rk-1, f = fl-1; r >= 0 && f >= 0; r--, f--)
	{
		result |= (BIT << (f + r*8));
		if (block & (BIT << (f + r * 8))) break;
	}
	return result;
}

int transform(U64 b, U64 magic, int bits)
{
	return (int)((b * magic) >> (64 - bits));
}

U64 findMagic(int sq, int bits, int bishop)
{
	U64 magic;
	U64 blocks[4096], atts[4096], used[4096];
	U64 mask = bishop ? bmask(sq) : rmask(sq);
 
	for (int i = 0; i < (1 << bits); i++)
	{
		blocks[i] = indexTo64(i, bits, mask);
		atts[i] = bishop ? batt(sq, blocks[i]) : ratt(sq, blocks[i]);
	}

	bool found = false;
	for (int k = 0; k < 100000000; k++)
	{
		magic = rand64Few();
		//if (POPCNT((mask * magic) & L(0xFF00000000000000)) < 6) continue;
		for (int i = 0; i < 4096; i++) used[i] = EMPTY;

		bool fail = false;
		for (int i = 0; !fail && i < (1 << bits); i++)
		{
			int j = transform(blocks[i], magic, bits);
			if (used[j] == EMPTY) used[j] = atts[i];
			else if (used[j] != atts[i]) fail = true;
		}

		if (!fail)
		{
#ifdef MAGICS_RECALC
			if (bishop) BMagic[sq] = magic;
			else        RMagic[sq] = magic;
#endif
			found = true;
			break;
		}
	}

	if (!found)
	{
		CON( "-- Failed --" );
		return EMPTY;
	}

	return magic;
}

int initMagics()
{
#ifdef MAGICS_RECALC
	CON( hex << uppercase << "const U64 RMagic[64] =\n{\n" );
	for (int i = 0; i < 64; i++)
	{
		CON( "    L(0x" << setfill('0') << setw(16)
			 << findMagic(i, RBits[i], 0) << ")" );
		CON( ((i < 63) ? "," : " ") );
		CON( " // " << SQ_OUT(i) << "\n" );
	}
	CON( "};\n\n" );
 
	CON( "const U64 BMagic[64] =\n{\n" );
	for (int i = 0; i < 64; i++)
	{
		CON( "    L(0x" << setfill('0') << setw(16)
			 << findMagic(i, BBits[i], 1) << ")" );
		CON( ((i < 63) ? "," : " ") );
		CON( " // " << SQ_OUT(i) << "\n" );
	}
	CON( "};\n\n" << nouppercase << dec );
#endif

	buildMagics();
 
	return 0;
}

void buildMagics()
{
	// Rooks ///////////////////////////////////////

	for (int sq = 0; sq < 64; sq++)
	{
		int bits = RBits[sq];
		U64 mask = rmask(sq);
		U64 magic = RMagic[sq];

		for (int i = 0; i < (1 << bits); i++)
		{
			U64 blocks = indexTo64(i, bits, mask);
			int j = transform(blocks, magic, bits);
			rAtt[sq][j] = ratt(sq, blocks);
		};

		rMagic[sq].mask = mask;
		rMagic[sq].magic = magic;
		rMagic[sq].shift = 64 - bits;
	}

	// Bishops /////////////////////////////////////

	for (int sq = 0; sq < 64; sq++)
	{
		int bits = BBits[sq];
		U64 mask = bmask(sq);
		U64 magic = BMagic[sq];

		for (int i = 0; i < (1 << bits); i++)
		{
			U64 blocks = indexTo64(i, bits, mask);
			int j = transform(blocks, magic, bits);
			bAtt[sq][j] = batt(sq, blocks);
		};

		bMagic[sq].mask = mask;
		bMagic[sq].magic = magic;
		bMagic[sq].shift = 64 - bits;
	}
}
