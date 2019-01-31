#include <iostream>
#include "search.h"
#include "board.h"

using namespace std;

bool console = true;

int main()
{
    Search * S = new Search;
    Board B;
    B.reset(S);
    B.print();


    system("pause");
    return 0;
}