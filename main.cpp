#include <iostream>
#include "search.h"
#include "board.h"
#include "eval.h"
#include "util.h"
#include "hash.h"
#include "piece.h"
#include "magics.h"
#include "consts.h"
#include "uci.h"

using namespace std;

bool console = true;
Search * S;
Board * B;
Eval * E;

int main()
{
    OUT(ENGINE_NAME << " v" << ENGINE_VERS << " chess engine by " << ENGINE_AUTH << " 2019\n\n");

    S = new Search;
    B = new Board;
    E = new Eval;

    init_lut();
	init_magics();
	init_arrays();
	init_pieces();
    //init_material();
	init_search();
	init_moves();
	//init_eval();
	init_hash();

    B->reset();
    B->print();

    while (parse_input() >= 0);

    system("pause");
    return 0;
}