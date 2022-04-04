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



// Transposition Table implementation:
//
// We use a C-stye array due to a much faster speed in read and
// write performance. The array contains TTEntry objects, which
// take 64 bits of space each (8 bytes).
//
// That means a cache size of 128MB will contain about 16M entries.


// initial hash size (~128MB)
extern uint32_t hash_size;


// number hash table entries
extern uint64_t hash_entries;


// no hash entry found constant
#define no_hash_entry 100000


// transposition table hash flags (node type)
#define hash_type_exact 0
#define hash_type_alpha 1
#define hash_type_beta  2


// TTEntry struct is the 8 bytes transposition table entry, defined as below:
//
// key        16 bit
// depth       8 bit
// type        8 bit
// value      32 bit
// best_move  16 bit
//
// Total size (per entry): 64 bits / 8 bytes
typedef struct {
    uint64_t key;   
    uint8_t  depth;      
    uint8_t  type;       
    int      value;
    int      best_move;
} TTEntry_t;

extern TTEntry_t *hash_table;



// initRandomKeys
//
// Functionality to initialize, generate, access and manipulate Zobrist
// hash keys as well as transposition tables.
void initRandomKeys();
uint64_t generateHashkey();
void clear_hash_table();
void init_hash_table(uint32_t);



#endif  //  TT_H
