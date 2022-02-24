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

#ifndef BOARD_H
#define BOARD_H


// Board, piece and position data structures and functions: this file includes
// the very definition of Bitboard (64-bit unsigned integer), the two sides
// (White and Black), and different square and piece data structures and
// functions.
//
// This file also include methods to operate with the board and printing it,
// as well as manipulating the position (including pieces, squares, side to
// to move, and other board/positional elements).



// Bitboard data type = unsigned long long (64-bit number)
#define Bitboard uint64_t



// Colors (and side to move): White, Black, NoColor
enum Side { White, Black, NoColor };



// List of board squares
enum BoardSquares {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};



// pawn attacks table [side][square]
extern Bitboard PawnAttacks[2][64];



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



void printBitboard(Bitboard);
std::string pretty(Bitboard);
Bitboard maskPawnAttacks(int, int);
void initLeaperAttacks();



#endif  // BOARD_H
