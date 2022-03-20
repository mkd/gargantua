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



// 'nodes' is a global variable holding the number of nodes analyzed
// or searched. It is used by negamax() but also other performance test
// functions such as perft().
extern uint64_t nodes;



// perft
//
// Verify move generation. All the leaf nodes up to the given depth are
// generated and counted.
static inline void perft(int depth)
{
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
        //saveBoard();
        assert(st != NULL);
        StateInfo newState = *st;
        newState.previous = st;
        st = &newState;



        // make move and, if illegal, skip to the next move
        if (!makeMove(MoveList.moves[move_count], AllMoves))
        {
            //takeBack();
            undoMove(MoveList.moves[move_count]);
            continue;
        }


        // call perft driver recursively
        perft(depth - 1);

        
        // take back
        //takeBack();
        undoMove(MoveList.moves[move_count]);
    }
}



// Functionality to search a position or perform an operation on the
// nodes of a given position.
void dperft(int);



#endif  //  SEARCH_H
