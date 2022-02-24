/*
  This file is part of Gargantua, a UCI chess engine derived from Chess0 which
  uses NNUE for evaluation. This engine has been inspired by BBC-1.4 (by Code
  Monkey King), Blunder (by Christian Dean) and Stockfish.
     
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
#include <string>
#include <sstream>
#include "board.h"
#include "bit.h"


using namespace std;



// PawnAttacks is an global variable (extern in board.h) seen by
// the entire program. Here it gets defined, so it can be used in
// other parts of the code:
Bitboard PawnAttacks[2][64];



// printBitboard
//
// Take a Bitboard and display it as a board representation on the standard
// output.
void printBitboard(Bitboard bb)
{
    cout << pretty(bb) << endl;
}



// pretty
//
// Take a Bitboard and return a string of how it should be printed.
string pretty(Bitboard bb)
{
    stringstream ss;

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // convert file & rank into square index
            int square = rank * 8 + file;

            // print ranks
            if (!file)
                ss << " " << 8 - rank << "  ";

            // print bit state (either 1 or 0)
            ss << " " << (getBit(bb, square) ? "1" : ".");
        }

        // print new line every rank
        ss << endl;
    }

    // print board files
    ss << endl << "     a b c d e f g h" << endl << endl;


    return ss.str();
}



// maskPawnAttacks
//
// Generate bitboard with all pawn attacks.
Bitboard maskPawnAttacks(int side, int square)
{
    // result attacks bitboard
    Bitboard attacks = 0ULL;

    // piece bitboard
    Bitboard bb = 0ULL;
    
    // set piece on board
    setBit(bb, square);
    
    // white pawns
    if (side == White)
    {
        if ((bb >> 7) & NotFileA_Mask) attacks |= (bb >> 7);
        if ((bb >> 9) & NotFileH_Mask) attacks |= (bb >> 9);
    }
    
    // black pawns
    else if (side == Black)
    {
        if ((bb << 7) & NotFileH_Mask) attacks |= (bb << 7);
        if ((bb << 9) & NotFileA_Mask) attacks |= (bb << 9);    
    }
    
    // return attack map
    return attacks;
}

// initLeaperAttacks
//
// Init leaper pieces attacks.
void initLeaperAttacks()
{
    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
    {
        // init pawn attacks
        PawnAttacks[White][square] = maskPawnAttacks(White, square);
        PawnAttacks[Black][square] = maskPawnAttacks(Black, square);
    }
}
