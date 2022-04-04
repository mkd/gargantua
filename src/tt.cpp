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

#include "bitboard.h"
#include "tt.h"
#include "position.h"



using namespace std;



// Zobrist hash keys for a given chess position:
Bitboard piece_keys[12][64];
Bitboard enpassant_keys[64];
Bitboard castle_keys[16];
Bitboard side_key;



// Transposition Table data structure and initializations

// initial hash size (~128MB)
uint32_t hash_size = 134217728;

// number hash table entries
uint64_t hash_entries = 0ULL;

// TT data structure
TTEntry_t *hash_table = nullptr;



// initRandomKeys
//
// Define and initialize a few 64-bit arrays that will serve the
// purpose of either building from scratch or incrementally updating
// a unique chess board position identifier, aka the hash key.
void initRandomKeys()
{
    // update pseudo random number state
    rng32_state = 1804289383;


    // init random piece keys
    for (int piece = P; piece <= k; piece++)
        for (int square = 0; square < 64; square++)
            piece_keys[piece][square] = rng64();

    
    // init random enpassant keys
    for (int square = 0; square < 64; square++)
        enpassant_keys[square] = rng64();

    
    // init castling keys
    for (int index = 0; index < 16; index++)
        castle_keys[index] = rng64();

        
    // init random side key
    side_key = rng64();
}



// generateHashkey
//
// Generate "almost" unique hash keys for every given position.
uint64_t generateHashkey()
{
    // final hash key
    uint64_t final_key = 0ULL;

    
    // temp piece bitboard copy
    Bitboard bb;

    
    // loop over piece bitboards
    for (int piece = P; piece <= k; piece++)
    {
        // init piece bitboard copy
        bb = bitboards[piece];
       

        // loop over the pieces within a bitboard
        while (bb)
            final_key ^= piece_keys[piece][popLsb(bb)];
    }

    
    // hash enpassant
    if (epsq != NoSq)
        final_key ^= enpassant_keys[epsq];

    
    // hash castling rights
    final_key ^= castle_keys[castle];

    
    // hash the side only if black is to move
    if (sideToMove == Black)
        final_key ^= side_key;
   

    // return generated hash key
    return final_key;
}



// clear_hash_table
//
// Clear the hash table containing the transposition table entries (TTEntry_t).
// This means all entries are reset to "zero" and made ready to be filled
// again.
void clear_hash_table()
{
    // init hash table entry pointer
    TTEntry_t *hash_entry;


    // loop over TT elements
    for (hash_entry = hash_table; hash_entry < hash_table + hash_entries; hash_entry++)
    {
        // reset TT inner fields
        hash_entry->key         = 0ULL;
        hash_entry->depth       = 0;
        hash_entry->type        = 0;
        hash_entry->value       = 0;
        hash_entry->best_move   = 0;
    }
}



// init_hash_table
//
// Dynamically allocate memory for the hash table (in MBytes).
void init_hash_table(uint32_t mb)
{
    // init hash size
    int hash_size = mb * 1024 * 1024;

    
    // init number of hash entries
    hash_entries =  hash_size / sizeof(TTEntry_t);


    // free hash table's dynamic memory
    if (hash_table != nullptr)
    {
        cout << "    Clearing hash memory..." << endl;
        free(hash_table);
    }

     
    // allocate memory
    hash_table = (TTEntry_t *) malloc(hash_entries * sizeof(TTEntry_t));


    // if allocation has failed
    if (hash_table == nullptr)
        cout << "    Couldn't allocate memory for hash table!" << endl;


    // if allocation succeeded, reset/clear the hash table entries
    else
    {
        clear_hash_table();
        cout << "Hash table initialized with " << hash_entries << " entries (";
        cout << mb << " MBytes)";
        cout << endl;
    }
}
