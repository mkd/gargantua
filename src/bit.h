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

#ifndef BIT_H
#define BIT_H

#include "board.h"


// Bitboard and bitwise operations:
//
// They can be defined both as macros (using #define) or as static inline
// functions. The difference is that, with macros, they are treated as pure
// test replacements by the preprocessor, so no typechecking is involved and
// it's more difficult to debug.
//
// However, static inline functions will only be *as fast* as macros when using
// optimizations (i.e., -Ofast or -O3), because the compiler will replace the
// function calls with inline code. So, remember to use optimizations!
//
// Below, there is a list of all the functions, plus how would they look like as 
// macros, in the comments.



// getBit
//
// #define getBit(b, pos) (b & (1ULL << pos))
static inline int getBit(Bitboard &b, int pos)
{
    return ((b >> pos) & 1ULL);
}



// setBit
//
// #define setBit(b, pos) (b |= (1ULL << pos))
static inline void setBit(Bitboard &b, int pos)
{
    b |= (1ULL << pos);
}



// clearBit
//
// #define clearBit(b, pos) (b &= ~(1ULL << pos))
static inline void clearBit(Bitboard &b, int pos)
{
    b &= ~(1ULL << pos);
}



// toggleBit
//
// #define toggleBit(b, pos) (b ^= (1ULL << pos))
static inline void toggleBit(Bitboard &b, int pos)
{
    b ^= (1ULL << pos);
}



#endif   // BIT_H
