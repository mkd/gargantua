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
#include "search.h"



using namespace std;



// Zobrist hash keys for a given chess position:
Bitboard piece_keys[12][64];
Bitboard enpassant_keys[64];
Bitboard castle_keys[16];
Bitboard side_key;



// Transposition Table data structure and initializations

// initial hash size (~128MB)
uint32_t hash_size = 134217728;

// current no. of total hash table entries
uint64_t hash_total_entries = 0ULL;

// no. of hash entries used
uint64_t hash_used = 0ULL;



// Global TT data structure
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



// TT::clear
//
// Clear the hash table containing the transposition table entries (TTEntry_t).
// This means all entries are reset to "zero" and made ready to be filled
// again.
void TT::clear()
{
    // init hash table entry pointer
    TTEntry_t *hash_entry;


    // loop over TT elements
    for (hash_entry = hash_table; hash_entry < hash_table + hash_total_entries; hash_entry++)
    {
        // reset TT inner fields
        hash_entry->key         = 0ULL;
        hash_entry->depth       = 0;
        hash_entry->type        = 0;
        hash_entry->value       = 0;
        hash_entry->best_move   = 0;
    }
}



// TT::init
//
// Dynamically allocate memory for the hash table (in MBytes).
void TT::init(uint32_t mb)
{
    // init hash size
    int hash_size = mb * 1024 * 1024;

    
    // init number of hash entries
    hash_total_entries =  hash_size / sizeof(TTEntry_t);


    // free hash table's dynamic memory
    if (hash_table != nullptr)
    {
        cout << "    Clearing hash memory..." << endl;
        free(hash_table);
    }

     
    // allocate memory
    hash_table = (TTEntry_t *) malloc(hash_total_entries * sizeof(TTEntry_t));


    // if allocation has failed
    if (hash_table == nullptr)
        cout << "    Couldn't allocate memory for hash table!" << endl;


    // if allocation succeeded, reset/clear the hash table entries
    else
    {
        TT::clear();

        cout << "Hash table initialized with " << hash_total_entries << " entries (";
        cout << mb << " MBytes)";
        cout << endl;
    }
}



// TT:probe
//
// Look up the current position in the transposition table and return is
// associated score. If the associated score is a fail-low, return alpha.
// If the associated score is a beta-cutoff, return beta. 
//
// In case the given position is not found, return no_hash_found.
int TT::probe(int alpha, int beta, int &best_move, int depth)
{
    // reliability checks
    assert(best_move != nullptr);


    // create a TT instance pointer to the hash entry in particular
    TTEntry_t *hash_entry = &hash_table[hash_key % hash_total_entries];

    
    // make sure we're dealing with the exact position we're looking for
    if (hash_entry->key == hash_key)
    {
        // check that the depth for the entry stored is the same or higher
        // (i.e., more accurate score)
        if (hash_entry->depth >= depth)
        {
            // extract stored score from TT entry
            int score = hash_entry->value;
           

            // if score is a mate, find the mating distance from the root node
            if (score < -MATESCORE)
                score += ply;
            else if (score > MATESCORE)
                score -= ply;
       

            // exact (PV node) score 
            if (hash_entry->type == hash_type_exact)
                return score;

            
            // the score is a fail-low node, return alpha
            if ((hash_entry->type == hash_type_alpha) && (score <= alpha))
                return alpha;

            
            // the score is a fail-high node, return beta
            if ((hash_entry->type == hash_type_beta) && (score >= beta))
                return beta;
        }
       

        // store best move
        best_move = hash_entry->best_move;
    }
   

    // if hash entry doesn't exist
    return no_hash_found;
}



// TT::save
//
// Populate the TTEntry with a new node's data, possibly overwriting an
// old position. Update is not atomic and can end up in race conditions.
void TT::save(int score, int best_move, int depth, int hash_type)
{
    // create a TT instance pointer to the hash entry in particular
    TTEntry_t *hash_entry = &hash_table[hash_key % hash_total_entries];


    // store the score independent from the actual path from root node
    if (score < -MATESCORE)
        score -= ply;
    else if (score > MATESCORE)
        score += ply;


    // if no collision, increment the counter of hash used
    if (hash_entry->depth == 0)
        hash_used++;


    // write hash entry data 
    hash_entry->key       = hash_key;
    hash_entry->value     = score;
    hash_entry->type      = hash_type;
    hash_entry->depth     = depth;
    hash_entry->best_move = best_move;
}
