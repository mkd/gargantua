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

#ifndef SEARCH_H
#define SEARCH_H

#include <cassert>

#include "movgen.h"



using namespace std;




// Default settings for search, when no settings are specified by the user:
//
// DEFAULT_SEARCH_DEPTH: depth limit used by the search
// DEFAULT_SEARCH_MOVETIME_MS: time limit used by the search (in milliseconds)
#define DEFAULT_SEARCH_DEPTH          16
#define DEFAULT_SEARCH_MOVETIME_MS  5000



// Search definitions, including alpha-beta bounds, mating scores, etc.
#define DRAWSCORE      0
#define MATEVALUE  49000
#define MATESCORE  48000



// 'nodes' is a global variable holding the number of nodes analyzed
// or searched. It is used by negamax() but also other performance test
// functions such as perft().
extern uint64_t nodes;



// Limits_t is a structure that holds the configuration of the search.
// This includes search depth, time to search, etc.
//
// The engine uses the global variable "limits" to set, edit and reset the
// search configuration throught the entire lifecycle.
typedef struct
{
    int wtime;
    int btime;
    int winc;
    int binc;
    int npmsec;
    int movetime;
    int movestogo;
    int depth;
    int mate;
    int perft;
    bool infinite;
    int nodes;
} Limits_t;

extern Limits_t Limits;



// perft
//
// Verify move generation. All the leaf nodes up to the given depth are
// generated and counted.
static inline void perft(int depth)
{
    // reliability checks
    assert(depth >= 0);


    // escape at leaf nodes and increment node count
    if (depth == 0)
    {
        nodes++;
        return;
    }

    
    // create move list instance
    MoveList_t MoveList;

    
    // generate moves
    generateMoves(MoveList);

    
    // loop over generated moves
    for (int move_count = 0; move_count < MoveList.count; move_count++)
    {   
        // preserve board state
        saveBoard();


        // make move and, if illegal, skip to the next move
        if (!makeMove(MoveList.moves[move_count], AllMoves))
        {
            takeBack();
            continue;
        }


        // call perft driver recursively
        perft(depth - 1);

        
        // undo move
        takeBack();
    }
}



// Functionality to search a position or perform an operation on the
// nodes of a given position.
void dperft(int);
void search();
void initSearch();
void resetLimits();



#endif  //  SEARCH_H
