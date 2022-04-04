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

#ifndef TT_H
#define TT_H



// Zobrist hash keys for a given chess position. Every position includes:
// - pieces and squares
// - enpassant square
// - castling rights
// - side to move
extern Bitboard piece_keys[12][64];
extern Bitboard enpassant_keys[64];
extern Bitboard castle_keys[16];
extern Bitboard side_key;



// initRandomKeys
//
// Functionality to initialize, generate, access and manipulate Zobrist
// hash keys as well as transposition tables.
void initRandomKeys();
uint64_t generateHashkey();



#endif  //  TT_H
