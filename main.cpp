#include <iostream>
#include "search.h"
#include "board.h"

using namespace std;

bool console = true;

int main()
{
    OUT("Eia v0.4 chess engine by Nick Kurgin 2019\n\n");

    Board B;
    B.reset();
    B.print();

    system("pause");
    return 0;
}