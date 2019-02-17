#include <iostream>
#include "search.h"
#include "board.h"
#include "util.h"
#include "magics.h"

using namespace std;

bool console = true;

int main()
{
    OUT("Eia v0.4 chess engine by Nick Kurgin 2019\n\n");
    initLut();
	initMagics();
	//initArrays();
	//initPieces();
    //initMaterial();
	//initSearch();
	//initMoves();
	//initEval();
	//initHash();

    Board B;
    B.reset();
    B.print();

    U64 occ = B.occ[0] | B.occ[1];
    print64(RATT(E4, occ));
    print64(BATT(E4, occ));

    system("pause");
    return 0;
}