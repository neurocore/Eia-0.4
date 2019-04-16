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
Hash * H;

int main()
{
    S = new Search;
    B = new Board;
    E = new Eval;
    H = new Hash;

    OUT(ENGINE_NAME << " v" << ENGINE_VERS << " chess engine by " << ENGINE_AUTH << " 2019\n\n");

    init_lut();
	init_magics();
	init_arrays();
	init_pieces();
	init_search();
	init_moves();
	init_eval();
	init_hash();

    B->reset();
    B->print();

    /*
    U64 mkey = MKEY_BR * 2 + MKEY_BB * 2 + MKEY_WP * 3 + MKEY_WQ;

    Material * m[2] = { E->mat_table + (mkey & 0xFFFFFFFF),
                        E->mat_table + (mkey >> 32)};
    int mat = m[WHITE]->val - m[BLACK]->val;
    int phase = m[B->wtm ^ 1]->phase;
    cout << m[WHITE]->val << " - " << m[BLACK]->val << " = " << mat << "\n";
    cout << "phase: {" << int(m[WHITE]->phase) << ", " << int(m[BLACK]->phase) << "}\n";*/

    while (parse_input() >= 0);
    return 0;
}