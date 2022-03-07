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

#include "bitboard.h"
#include "position.h"



// genMoves
//
// Generate all pseudo-legal moves for the current position.
//
// TODO: compare performance vs. inline
void genMoves()
{
    // define source & target squares
    int fromSq, toSq;

    
    // current piece's Bitboard copy & it's attacks
    Bitboard bb, attacks;

    
    // loop over all the Bitboards (current position)
    for (int piece = P; piece <= k; piece++)
    {
        // init piece bitboard copy
        bb = bitboards[piece];
        
        // generate white pawns & white king castling moves
        if (sideToMove == White)
        {
        
        }
        
        // generate black pawns & black king castling moves
        else
        {
        
        }
        
        // genarate knight moves
        
        // generate bishop moves
        
        // generate rook moves
        
        // generate queen moves
        
        // generate king moves
    }
}
