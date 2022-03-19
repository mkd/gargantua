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

#ifndef POSITION_H
#define POSITION_H

#include <map>
#include <cstring>



// List of useful FEN positions used for testing and debbuging purposes
// @see https://www.chessprogramming.org/Perft_Results
#define FENPOS_EMPTYBOARD "8/8/8/8/8/8/8/8 b - - "
#define FENPOS_STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define FENPOS_KIWIPETE "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "
#define FENPOS_POS3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "
#define FENPOS_POS4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define FENPOS_POS5 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
#define FENPOS_POS6 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"




// ASCII piece strings lookup table
static const std::array<std::string, 12> PieceStr
{
    "P", "N", "B", "R", "Q", "K",
    "p", "n", "b", "r", "q", "k",
};



// Convert ASCII character pieces to encoded constants
static std::map<unsigned char, int> PieceConst = {
    {'P', P},
    {'N', N},
    {'B', B},
    {'R', R},
    {'Q', Q},
    {'K', K},
    {'p', p},
    {'n', n},
    {'b', b},
    {'r', r},
    {'q', q},
    {'k', k}
};



// Array of promoted pieces, encoded from integer to character
static std::map<int, unsigned char> PromoPieces = {
    {0, '\0'},
    {Q, 'q'},
    {R, 'r'},
    {B, 'b'},
    {N, 'n'},
    {q, 'q'},
    {r, 'r'},
    {b, 'b'},
    {n, 'n'}
};



// A chess position is defined by the following elements:
//
// 1. A set of 12 bitboards with all the piece occupancies
// 2. The side to move
// 3. The enpassant capture square
// 4. The castling rights
extern Bitboard bitboards[12];
extern Bitboard occupancies[3];
extern int sideToMove;
extern int epsq;
extern int castle;



// Flag to indicate whether the board should be displayed from White's
// perspective (false) or Black's perspective (true).
extern bool flip;



// Castling rights binary encoding
/*
    bin  dec
    
   0001    1  White King can castle to the king side
   0010    2  White King can castle to the queen side
   0100    4  Black King can castle to the king side
   1000    8  Black King can castle to the queen side


   Examples:

   1111       both sides an castle both directions
   1001       black king => queen side
              white king => king side
*/
enum CastlingRights { wk = 1, wq = 2, bk = 4, bq = 8 };



// Functionality to handle a position on the chess board, including
// resting the board to its initial status, printing the board and
// parsing positions in FEN notation.
void resetBoard();
void printBoard();
void setPosition(const std::string &);



#endif  //  POSITIION_H
