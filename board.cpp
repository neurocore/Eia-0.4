#include <string>
#include "board.h"

void Board::clear()
{
    wtm = 1;
    for (int i = 0; i < PIECE_N; i++) piece[i] = EMPTY;
    for (int i = 0; i < COLOR_N; i++) occ[i] = EMPTY;
    for (int i = 0; i < SQUARE_N; i++) sq[i] = NOP;
}

void Board::reset(Search * S)
{
    fromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", S);
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

struct Node {};
struct Eval {};

#define SET_PIECE(s, p) { piece[p] |= (BIT << (s)); \
                          this->sq[s] = p;            \
                                                    }
                          //N->mat += EV->mat[p];        \
                          //N->pst += EV->pst[p][s]; 

void Board::fromFen(const char * fen, Search * S)
{
    clear();
    /*Node * N = S->node;
    Eval * EV = S->eval;*/
    //N->mat = 0;
    //N->pst.clear();
    int i = 0, sq = A8;

    while (char ch = fen[i++])
    {
        bool shift = true;
        switch(ch)
        {
            case 'p': SET_PIECE(sq, BP); break;
            case 'P': SET_PIECE(sq, WP); break;
            case 'n': SET_PIECE(sq, BN); break;
            case 'N': SET_PIECE(sq, WN); break;
            case 'b': SET_PIECE(sq, BB); break;
            case 'B': SET_PIECE(sq, WB); break;
            case 'r': SET_PIECE(sq, BR); break;
            case 'R': SET_PIECE(sq, WR); break;
            case 'q': SET_PIECE(sq, BQ); break;
            case 'Q': SET_PIECE(sq, WQ); break;
            case 'k': SET_PIECE(sq, BK); break;
            case 'K': SET_PIECE(sq, WK); break;

            case '/': continue;
            case ' ': sq = -1; break;

            default:

                shift = false;
                if (ch > '0' && ch < '9') sq += ch - '0';
        }

        if (shift) sq++;

        if (!(sq & 7)) // End of row
        {
            sq -= 16;
            if (sq < 0) break;
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
    /*N->castling = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == '-' || ch == ' ') break;
        if (ch == 'k') N->castling |= C_BK;
        if (ch == 'q') N->castling |= C_BQ;
        if (ch == 'K') N->castling |= C_WK;
        if (ch == 'Q') N->castling |= C_WQ;
    }*/

    // En passant target square
    /*N->ep = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == '-' || ch == ' ') break;
        if (ch >= 'a' && ch <= 'h') N->ep += SQ(ch - 'a', 0);
        if (ch >= 'A' && ch <= 'H') N->ep += SQ(ch - 'A', 0);
        if (ch == '3' || ch == '6') N->ep += SQ(0, ch - '1');
    }
    if (Y(N->ep) != 2 && Y(N->ep) != 5) N->ep = 0;*/

    // Halfmove clock
    /*N->fifty = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == ' ') break;
        if (ch >= '0' && ch <= '9')
        {
            N->fifty *= 10;
            N->fifty += ch - '0';
        }
    }*/

    // Full move counter
   /* S->movecnt = 0;
    while (fen[i] == ' ') i++;
    while (char ch = fen[i++])
    {
        if (ch == ' ') break;
        if (ch >= '0' && ch <= '9')
        {
            S->movecnt *= 10;
            S->movecnt += ch - '0';
        }
    }

    S->movecnt *= 2;
    S->movecnt -= wtm;
    S->movecnt--;*/

    // Occupied
    occ[0] = occ[1] = EMPTY;
    for (int i = BP; i < NOP; i += 2)
    {
        occ[0] |= piece[i];
        occ[1] |= piece[i + 1];
    }

    /*N->hash = calcHash(N);
    N->phash = calcPHash(N);
    S->threefold[S->movecnt] = N->hash;*/
}