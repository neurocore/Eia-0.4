#include <windows.h>
#include <conio.h>

#include <iostream>
#include <sstream>
#include <string>
#include "types.h"
#include "board.h"
#include "moves.h"
#include "consts.h"
#include "search.h"

using namespace std;

bool try_parse_int(const string & s, int & r)
{
    istringstream ss(s);
    ss >> r;
    return !ss.fail() && ss.eof();
}

int parseInput()
{
	string input;
	getline(cin, input);
	istringstream sin(input);

	//INP(input); // Logging input

	string command;
	sin >> command;

    if (command == "position")
	{
		string part, rest;
		sin >> part;

		bool moves = false;
		if (part == "fen")
		{
			string fen, fenpart;

			while (!sin.eof())
			{
				sin >> fenpart;
				if (fenpart == "\n") break;
				if (fenpart == "moves")
				{
					moves = true;
					break;
				}
				fen += fenpart + " ";
			}

			B->clear();
			B->from_fen(fen.c_str());
			CON( "Fen: " << fen << "\n\n" );
		}
		else if (part == "startpos")
		{
			B->reset();
		}

		if (!moves)
		{
			sin >> part;
			if (part == "moves") moves = true;
		}

		if (moves)
		{
			//Node * N = S->node;
			Move move;
			do
			{
				sin >> part;
				if (part == "\n") break;

				if (B->try_parse(part, move))
				{
					if (B->is_legal(move)) B->make(move, true);
					else
					{
						CON("Not legal move \"" << part << "\"\n");
						break;
					}
				}
				else CON("Can't parse move \"" << part << "\"\n");

				//move.print(true);
				//engine->board->print();
				//CON("________________\n\n");
			}
			while (!sin.eof());
		}

		B->print();
	}
	else if (command == "go")
	{
		int times[2] = {TIME_DEFAULT, TIME_DEFAULT};
		S->infinite = false;
		string part;
		while (!sin.eof())
		{
			sin >> part;
			if (part == "wtime") sin >> times[1];
			if (part == "btime") sin >> times[0];
			if (part == "infinite") S->infinite = true;
		}
		S->color = B->wtm;
		S->think_time = times[S->color];

		think();
	}
    else if (command == "stop")
	{
		S->status = Waiting;
	}
    else if (command == "ucinewgame")
	{
		//S->clear();
	}
    else if (command == "setoption")
	{
	}
    else if (command == "isready")
	{
		OUT("readyok\n");
        FLUSH;
	}
	else if (command == "uci")
	{
		console = false;
		
		OUT("id name " << ENGINE_NAME << " v" << ENGINE_VERS << "\n");
		OUT("id author " << ENGINE_AUTH << "\n");
		OUT("uciok\n");
        FLUSH;
	}
	else if (command == "quit")
	{
		return -1;
	}
	else if (command == "perft")
	{
		string part;
		sin >> part;

		int num;
		if (!try_parse_int(part, num)) num = 1;
		perft_root(num);
	}
	else if (command == "bench")
	{
		bench();
	}
	else
	{
		CON("Unknown command \"" << command << "\"\n");
	}
	return 0;
}

bool input_available()
{
	//if (stdin->_cnt > 0) return 1;

	DWORD mode;
	static HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	static BOOL console = GetConsoleMode(hInput, &mode);

	if (!console)
	{
		DWORD totalBytesAvail;
		if (!PeekNamedPipe(hInput, 0, 0, 0, &totalBytesAvail, 0)) return true;
		return totalBytesAvail;
	}
	else return _kbhit();
}
