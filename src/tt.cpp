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
#include "tt.h"



// Zobrist hash keys for a given chess position:
Bitboard piece_keys[12][64];
Bitboard enpassant_keys[64];
Bitboard castle_keys[16];
Bitboard side_key;



// initRandomKeys
//
// Define and initialize a few 64-bit arrays that will serve the
// purpose of either building from scratch or incrementally updating
// a unique chess board position identifier, aka the hash key.
void initRandomKeys()
{
    // update pseudo random number state
    rng32_state = 1804289383;


    // loop over the pieces
    for (int piece = P; piece <= k; piece++)
    {
        // loop over the board squares
        for (int square = 0; square < 64; square++)
            // init random piece keys
            piece_keys[piece][square] = rng64();
    }

    
    // loop over the board squares
    for (int square = 0; square < 64; square++)
        // init random enpassant keys
        enpassant_keys[square] = rng64();

    
    // loop over castling keys
    for (int index = 0; index < 16; index++)
        // init castling keys
        castle_keys[index] = rng64();

        
    // init random side key
    side_key = rng64();
}
