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
#define INFINITY   50000

#define MOVESCORE_PROMO_QUIET  10



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



// Most Valuable Victim / Less Valuable Attacker (MVV/LVA) lookup table
/*
                          
    (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600
*/

// MVV LVA [attacker][victim]
static constexpr int mvv_lva[12][12] =
{
 	{105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600},

	{105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600}
};



// Functionality to search a position or perform an operation on the
// nodes of a given position.
void dperft(int);
void search();
int  qsearch(int, int);
void initSearch();
void resetLimits();
void sortMoves(MoveList_t &);
void printMoveScores(MoveList_t &);



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



// scoreMove
//
// Assign a score to a move.
static inline int scoreMove(int move)
{
    // score capture move
    if (getMoveCapture(move))
    {
        // init target piece
        int target_piece = P;
        int toSq = getMoveTarget(move);

        
        // pick up bitboard piece index ranges depending on side
        int start_piece, end_piece;

        
        // pick up side to move
        if (sideToMove == White)
        {
            start_piece = p;
            end_piece = k;
        }
        else
        {
            start_piece = P;
            end_piece = K;
        }

        
        // loop over the opponent's bitboards
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
        {
            // if there's a piece on the target square
            if (getBit(bitboards[bb_piece], toSq))
            {
                // remove it from corresponding bitboard
                target_piece = bb_piece;
                break;
            }
        }
                
        // score move by MVV LVA lookup [source piece][target piece]
        return mvv_lva[getMovePiece(move)][target_piece];
    }


    // quiet promotions are also scored
    else if (getPromo(move))
    {
        return MOVESCORE_PROMO_QUIET;
    }
    
    // score quiet move
    //else
    //{ }
   

    return 0;
}



#endif  //  SEARCH_H
