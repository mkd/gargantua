/*
  This file is part of Gargantua, a UCI chess engine with NNUE evaluation
  derived from Chess0, and inspired by Code Monkey King's bbc-1.4.
     
  Copyright (C) 2022 Claudio M. Camacho
 
  Gargantua is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  Gargantua is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <map>

#include "bitboard.h"
#include "position.h"
#include "tt.h"



using namespace std;



// A position is defined by the following elements:
//
// 1. A set of 12 bitboards with all the piece occupancies
// 2. The side to move
// 3. The enpassant capture square
// 4. The castling rights
// 5. The 50-move rule (50 moves without captures, pawn moves nor promotions)
// 6. A ply counter (to separate root moves from the rest)
thread_local Bitboard bitboards[12];
thread_local Bitboard occupancies[3];
thread_local int sideToMove = White;
thread_local int epsq = NoSq; 
thread_local int castle;
thread_local int fifty = 0;
thread_local int ply = 0;



// Chess position's (almost) unique hash key
thread_local uint64_t hash_key = 0ULL;



// Flag to indicate whether the board should be displayed from White's
// perspective (false) or Black's perspective (true).
bool flip = false;



// resetBoard
//
// Reset the board variables, set the pieces back to start position, etc.
void resetBoard()
{
    // reset board position and occupancies
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));

    
    // reset game state variables
    sideToMove = White;
    epsq       = NoSq;
    castle     = 0;
    flip       = false;
   

    // reset repetition index
    //repetition_index = 0;


    // reset fifty move rule counter
    fifty = 0;


    // reset repetition table
    //memset(repetition_table, 0ULL, sizeof(repetition_table));
}



// setPosition
//
// Initialize the position with the given FEN string.
//
// This function is not very robust - make sure that input FENs are correct,
// this is assumed to be the responsibility of the GUI.
//
// @see https://github.com/official-stockfish/Stockfish/blob/master/src/position.cpp
void setPosition(const string &fenStr)
{
/*
    A FEN string defines a particular position using only the ASCII character set.

    A FEN string contains six fields separated by a space. The fields are:

    1) Piece placement (from white's perspective). Each rank is described, starting
       with rank 8 and ending with rank 1. Within each rank, the contents of each
       square are described from file A through file H. Following the Standard
       Algebraic Notation (SAN), each piece is identified by a single letter taken
       from the standard English names. White pieces are designated using upper-case
       letters ("PNBRQK") whilst Black uses lowercase ("pnbrqk"). Blank squares are
       noted using digits 1 through 8 (the number of blank squares), and "/"
       separates ranks.

    2) Active color. "w" means white moves next, "b" means black.

    3) Castling availability. If neither side can castle, this is "-". Otherwise,
       this has one or more letters: "K" (White can castle kingside), "Q" (White
       can castle queenside), "k" (Black can castle kingside), and/or "q" (Black
       can castle queenside).

    4) En passant target square (in algebraic notation). If there's no en passant
       target square, this is "-". If a pawn has just made a 2-square move, this
       is the position "behind" the pawn. Following X-FEN standard, this is recorded only
       if there is a pawn in position to make an en passant capture, and if there really
       is a pawn that might have advanced two squares.

    5) Halfmove clock. This is the number of halfmoves since the last pawn advance
       or capture. This is used to determine if a draw can be claimed under the
       fifty-move rule.

    6) Fullmove number. The number of the full move. It starts at 1, and is
       incremented after Black's move.
*/

    unsigned char col, row, token;
    int sq = a8;
    map<unsigned char, int>::iterator it;
    istringstream ss(fenStr);
    ss >> noskipws;


    // reset board status
    resetBoard();


    // 1. Piece placement
    int rank = 0, file = 0;
    while ((ss >> token) && !isspace(token))
    {
        if (isdigit(token))
        {
            file += (token - '0');
            sq += (token - '0');
        }
        else if (token == '/')
        {
            rank++;
            file = 0;
            sq = rank * 8 + file;
        }
        else if ((it = PieceConst.find(token)) != PieceConst.end())
        {
            sq = rank * 8 + file;
            setBit(bitboards[PieceConst[token]], sq);
            sq++;
            file++;
        }
    }


    // 2. Side to move
    ss >> token;
    sideToMove = (token == 'w' ? White : Black);
    ss >> token;


    // 3. Castling availability
    while ((ss >> token) && !isspace(token))
    {
        switch (token)
        {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            case '-': break;
        }
    }


    // 4. Enpassant square
    // Ignore if square is invalid or not on side to move relative rank 6.
    if (   ((ss >> col) && (col >= 'a' && col <= 'h'))
        && ((ss >> row) && (row == (sideToMove == White ? '6' : '3'))))
    {
        // parse enpassant file & rank
        int file = col - 'a';
        int rank = 8 - (row - '0');

        // set enpassant only if sideToMove matches enpanssant square
        if (   ((sideToMove == White) && (rank == 2))
            || ((sideToMove == Black) && (rank == 5)))
        {
            epsq = rank * 8 + file;
        }
    }
    else
    {
        epsq = NoSq;
    }


    // 5-6. Halfmove clock and fullmove number
    ss >> skipws >> fifty;


    // populate white occupancy bitboard
    for (int piece = P; piece <= K; piece++)
        occupancies[White] |= bitboards[piece];
   

    // populate white occupancy bitboard
    for (int piece = p; piece <= k; piece++)
        occupancies[Black] |= bitboards[piece];
   

    // init all occupancies
    occupancies[Both] |= occupancies[White];
    occupancies[Both] |= occupancies[Black];
   

    // init hash key
    hash_key = generateHashkey();
}



// getFEN
//
// Return a FEN representation of the current position.
string getFEN()
{
    int emptyCnt = 0;
    int fromSq, rank, file;
    ostringstream ss;


    // 64-square board where to place the pieces for processing
    int board[8][8] = {
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1}
    };


    // serialize all pieces into a board structure
    Bitboard bb = occupancies[Both];
    while (bb)
    {
        fromSq = popLsb(bb);

        if (SqBB[fromSq] & bitboards[P])
            board[fromSq % 8][fromSq / 8] = P;
        else if (SqBB[fromSq] & bitboards[N])
            board[fromSq % 8][fromSq / 8] = N;
        else if (SqBB[fromSq] & bitboards[B])
            board[fromSq % 8][fromSq / 8] = B;
        else if (SqBB[fromSq] & bitboards[R])
            board[fromSq % 8][fromSq / 8] = R;
        else if (SqBB[fromSq] & bitboards[Q])
            board[fromSq % 8][fromSq / 8] = Q;
        else if (SqBB[fromSq] & bitboards[K])
            board[fromSq % 8][fromSq / 8] = K;
        else if (SqBB[fromSq] & bitboards[p])
            board[fromSq % 8][fromSq / 8] = p;
        else if (SqBB[fromSq] & bitboards[n])
            board[fromSq % 8][fromSq / 8] = n;
        else if (SqBB[fromSq] & bitboards[b])
            board[fromSq % 8][fromSq / 8] = b;
        else if (SqBB[fromSq] & bitboards[r])
            board[fromSq % 8][fromSq / 8] = r;
        else if (SqBB[fromSq] & bitboards[q])
            board[fromSq % 8][fromSq / 8] = q;
        else if (SqBB[fromSq] & bitboards[k])
            board[fromSq % 8][fromSq / 8] = k;
    }


    // translate pieces on the board into FEN notation
    for (rank = 0; rank <= 7; ++rank)
    {
        for (file = 0; file <= 7; file++)
        {
            for (emptyCnt = 0; (file <= 7) && (board[file][rank] == -1); file++)
                emptyCnt++;

            if (emptyCnt)
                ss << emptyCnt;

            if (file <= 7)
               ss << PieceStr[board[file][rank]];
        }

        if (rank < 7)
            ss << '/';
    }


    // side to move
    ss << (sideToMove == White ? " w " : " b ");


    // castling rights
    if (castle & wk)
        ss << 'K';

    if (castle & wq)
        ss << 'Q';

    if (castle & bk)
        ss << 'k';

    if (castle & bq)
        ss << 'q';

    if (!castle)
        ss << '-';

    ss << " ";


    // enpassant square
    if (epsq && (epsq == NoSq))
        ss << "-";
    else
        ss << SquareToCoordinates[epsq];

    ss << " ";


    // fifty rule
    ss << fifty << " ";


    // ply
    ss << 1 + (ply - (sideToMove == Black)) / 2;


    // return FEN string
    return ss.str();
}



// printBoard
//
// Convert the internal representation of the board into a human-readable string
// (capable of being shown and represented as a Board in ASCII) and show it on
// the screen.
void printBoard()
{
    cout << endl << endl;
    cout << "    +----+----+----+----+----+----+----+----+" << endl;


    // loop over board squares
    for (int rank = 0; rank < 8; rank++)
    {
        // show board from Black's perspective
        if (flip)
            cout << setw(3) << rank + 1 <<  " |";
        // show board form White's perspectivwe
        else
            cout << setw(3) << 8 - rank <<  " |";


        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // convert file & rank into square index:
            // flip takes care of the board's perspective (Black or White)
            int square = NoSq;
            if (flip)
                square = ((7 - rank) * 8) + (7 - file);
            else
                square = (rank * 8) + file;


            // loop over all piece bitboards
            int piece = -1;
            Side color = NoColor;
            for (int bb_piece = P; bb_piece <= k; bb_piece++)
            {
                if (getBit(bitboards[bb_piece], square))
                    piece = bb_piece;

                switch (piece)
                {
                    case p: 
                    case n: 
                    case b: 
                    case r: 
                    case q: 
                    case k: color = Black;
                            break;
                    default: color = White;
                }
            }


            // print bit state (either 1 or 0)
            cout << " " << ((piece == -1) ? " " : PieceStr[piece]);
            cout << ((color == White) ? " " : "*") << " |";
        }


        // print new line every rank
        cout << endl << "    +----+----+----+----+----+----+----+----+" << endl;
    }


    // print board files
    cout << "      a    b    c    d    e    f    g    h" << endl << endl;


    // print board status
    cout << "  Fen:    " << getFEN() << endl;
    cout << "  Key:    " << hex << uppercase << hash_key << endl;
    cout << "  Side:   " << ((sideToMove == White) ? "White" : "Black") << endl;
    cout << "  Epsq:   " << ((epsq != NoSq) ? SquareToCoordinates[epsq] : "-") << endl;
    cout << "  Castle: " << ((castle & wk) ? "K" : "-") <<
                            ((castle & wq) ? "Q" : "-") <<
                            ((castle & bk) ? "k" : "-") <<
                            ((castle & bq) ? "q" : "-") << endl;

    // reset formating for the next time
    cout << resetiosflags(std::cout.flags());
    cout << endl;
}
