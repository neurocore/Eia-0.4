#include <sstream>
#include <string>
#include "board.h"
#include "search.h"
#include "magics.h"
#include "piece.h"
#include "util.h"
#include "hash.h"
#include "eval.h"

using namespace std;

void Board::clear()
{
    wtm = 1;
    for (int i = 0; i < PIECE_N; i++) piece[i] = EMPTY;
    for (int i = 0; i < COLOR_N; i++) occ[i] = EMPTY;
    for (int i = 0; i < SQUARE_N; i++) sq[i] = NOP;
}

void Board::reset()
{
    fromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::print()
{
    const string pieceChar = "pPnNbBrRqQkK..";
    const string toMove = wtm ? "<W>" : "<B>";
    U64 occupied = occ[BLACK] | occ[WHITE];

    for (int y = 7; y >= 0; y--)
    {
        CON(RANKS[y] << "|");

        for (int x = 0; x < 8; x++)
        {
            U64 bit = BIT << SQ(x, y);

            if (bit & occupied)
            {
                char ch = '?';
                int color = !!(bit & occ[BLACK]);
                for (int k = BP + color; k < PIECE_N; k += 2)
                {
                    if (bit & piece[k])
                    {
                        ch = pieceChar[k];
                        break;
                    }
                }
                CON(ch);
            }
            else
                CON('.');
        }

        if (y == 0) CON(" " << toMove);
        CON("\n");
    }
    CON(" +--------\n  " << FILES << "\n\n");
}

#define SET_PIECE(s, p) { piece[p] |= (BIT << (s));  \
                          this->sq[s] = p;           \
                                                     }
                          //state.pst += E->pst[p][s]; \

void Board::fromFen(const char * fen)
{
    clear();
    //Eval * E = S->eval;
    state.pst = 0;
    int i = 0, square = A8;

    while (char ch = fen[i++])
    {
        bool shift = true;
        switch(ch)
        {
            case 'p': SET_PIECE(square, BP); break;
            case 'P': SET_PIECE(square, WP); break;
            case 'n': SET_PIECE(square, BN); break;
            case 'N': SET_PIECE(square, WN); break;
            case 'b': SET_PIECE(square, BB); break;
            case 'B': SET_PIECE(square, WB); break;
            case 'r': SET_PIECE(square, BR); break;
            case 'R': SET_PIECE(square, WR); break;
            case 'q': SET_PIECE(square, BQ); break;
            case 'Q': SET_PIECE(square, WQ); break;
            case 'k': SET_PIECE(square, BK); break;
            case 'K': SET_PIECE(square, WK); break;

            case '/': continue;
            case ' ': square = -1; break;

            default:

                shift = false;
                if (ch > '0' && ch < '9') square += ch - '0';
        }

        if (shift) square++;

        if (!(square & 7)) // End of row
        {
            square -= 16;
            if (square < 0) break;
        }
    }

    // White to move
    wtm = 2;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == 'w') wtm = 1;
        if (ch == 'b') wtm = 0;
        if (wtm < 2) break;
    }

    // Castling rights
    state.castling = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == '-' || ch == ' ') break;
        if (ch == 'k') state.castling |= C_BK;
        if (ch == 'q') state.castling |= C_BQ;
        if (ch == 'K') state.castling |= C_WK;
        if (ch == 'Q') state.castling |= C_WQ;
    }

    // En passant target square
    state.ep = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == '-' || ch == ' ') break;
        if (ch >= 'a' && ch <= 'h') state.ep += SQ(ch - 'a', 0);
        if (ch >= 'A' && ch <= 'H') state.ep += SQ(ch - 'A', 0);
        if (ch == '3' || ch == '6') state.ep += SQ(0, ch - '1');
    }
    if (Y(state.ep) != 2 && Y(state.ep) != 5) state.ep = 0;

    // Halfmove clock
    state.fifty = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == ' ') break;
        if (ch >= '0' && ch <= '9')
        {
            state.fifty *= 10;
            state.fifty += ch - '0';
        }
    }

    // Full move counter -> TODO
    int movecnt = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == ' ') break;
        if (ch >= '0' && ch <= '9')
        {
            movecnt *= 10;
            movecnt += ch - '0';
        }
    }
    movecnt *= 2;
    movecnt -= wtm;
    movecnt--;

    // Occupied
    occ[0] = occ[1] = EMPTY;
    for (int i = BP; i < PIECE_N; i += 2)
    {
        occ[0] |= piece[i];
        occ[1] |= piece[i + 1];
    }

    /*N->hash = calcHash(N);
    N->phash = calcPHash(N);
    S->threefold[S->movecnt] = N->hash;*/
}