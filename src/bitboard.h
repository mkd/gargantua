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

#ifndef BITBOARD_H
#define BITBOARD_H



#include "definitions.h"



// board square masks, including different files, ranks and square groups
constexpr Bitboard AllSquares    = ~Bitboard(0);
constexpr Bitboard DarkSquares   = 0xAA55AA55AA55AA55ULL;
constexpr Bitboard LightSquares  = ~(0xAA55AA55AA55AA55ULL);

constexpr Bitboard FileA_Mask = 0x0101010101010101ULL;
constexpr Bitboard FileB_Mask = FileA_Mask << 1;
constexpr Bitboard FileC_Mask = FileA_Mask << 2;
constexpr Bitboard FileD_Mask = FileA_Mask << 3;
constexpr Bitboard FileE_Mask = FileA_Mask << 4;
constexpr Bitboard FileF_Mask = FileA_Mask << 5;
constexpr Bitboard FileG_Mask = FileA_Mask << 6;
constexpr Bitboard FileH_Mask = FileA_Mask << 7;

constexpr Bitboard Rank1_Mask = 0xFF;
constexpr Bitboard Rank2_Mask = Rank1_Mask << (8 * 1);
constexpr Bitboard Rank3_Mask = Rank1_Mask << (8 * 2);
constexpr Bitboard Rank4_Mask = Rank1_Mask << (8 * 3);
constexpr Bitboard Rank5_Mask = Rank1_Mask << (8 * 4);
constexpr Bitboard Rank6_Mask = Rank1_Mask << (8 * 5);
constexpr Bitboard Rank7_Mask = Rank1_Mask << (8 * 6);
constexpr Bitboard Rank8_Mask = Rank1_Mask << (8 * 7);

constexpr Bitboard NotFileA_Mask  = 18374403900871474942ULL;
constexpr Bitboard NotFileH_Mask  = 9187201950435737471ULL;
constexpr Bitboard NotFileHG_Mask = 4557430888798830399ULL;
constexpr Bitboard NotFileAB_Mask = 18229723555195321596ULL;




// Functions for initializing, printing and manipulating Bitboard data
// structures and information:
void printBitboard(Bitboard);
std::string pretty(Bitboard);
Bitboard genRookAttacks(int, Bitboard);
void initLeaperAttacks();




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



#endif  // BITBOARD_H
