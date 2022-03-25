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
#include <iomanip>
#include <chrono>
#include <cassert>

#include "search.h"
#include "eval.h"



// 'nodes' is a global variable holding the number of nodes analyzed
// or searched. It is used by negamax() but also other performance test
// functions such as perft().
uint64_t nodes = 0ULL;



// Limits holds the configuration of the search: time, search depth, etc.
Limits_t Limits;



// TODO: ditch when implementing iterative-deepening framework
int bestmove;



// initSearch
//
// Intialize the search parameters to the default ones.
void initSearch()
{
    resetLimits();
}



// resetLimits
//
// Reset all search limits to their initial configuration.
void resetLimits()
{
    Limits.wtime = 0;
    Limits.btime = 0;
    Limits.winc = 0;
    Limits.binc = 0;
    Limits.npmsec = 0;
    Limits.movetime = 0;
    Limits.movestogo = 0;
    Limits.depth = DEFAULT_SEARCH_DEPTH;
    Limits.mate = 0;
    Limits.perft = 0;
    Limits.infinite = 0;
    Limits.nodes = 0;
}



// negamax
//
// Main alphabeta algorithm (Negamax) which relies on a Principal Variation
// search. This algorithm uses the following steps:
//
//  1. static null move pruning
//  2. null move pruning
//  3. razoring
//  2. sort moves (score based on historic appearance, capture gain, etc)
//  3. look up move hash (from previous searches)
//  4. late move reductions (LMR)
//  8. start full search
//
// The score returned by the algorithm is always from calling qsearch().
int negamax(int alpha, int beta, int depth)
{
    // reliability check
    assert(depth >= 0);


    // leaf node: return static evaluation
    if (depth == 0)
        return qsearch(alpha, beta);

    
    // increment nodes count
    nodes++;


    // is king in check
    int inCheck = isSquareAttacked((sideToMove == White) ? ls1b(bitboards[K]) : 
                                                           ls1b(bitboards[k]),
                                                           sideToMove ^ 1);

    
    // number of legal moves found
    int legal = 0;

    
    // best move so far
    int best_sofar;

    
    // old value of alpha
    int old_alpha = alpha;


    // create move list instance
    MoveList_t MoveList;
   

    // generate moves
    generateMoves(MoveList);

    
    // loop over moves within a movelist
    for (int count = 0; count < MoveList.count; count++)
    {
        // preserve board state
        saveBoard();
       

        // increment ply
        ply++;
       

        // make sure to make only legal moves
        if (!makeMove(MoveList.moves[count], AllMoves))
        {
            // decrement ply
            ply--;

            // undo move
            takeBack();
            
            // skip to next move
            continue;
        }


        // increment legal moves
        legal++;

        
        // get score for the current move
        int score = -negamax(-beta, -alpha, depth - 1);

        
        // decrement ply
        ply--;


        // undo move
        takeBack();

        
        // fail-hard beta cutoff
        if (score >= beta)
        {
            // node (move) fails high
            return beta;
        }

        
        // found a better move (improves alpha)
        if (score > alpha)
        {
            // PV node (move)
            alpha = score;
            
            // if root move
            if (ply == 0)
                // associate best move with the best score
                best_sofar = MoveList.moves[count];
        }
    }


    // checkmate or stalemate detection
    if (legal == 0)
    {
        // king is in check: return mating score (closest distance to mate)
        if (inCheck)
            return -MATEVALUE + ply;
        
        // king not in check: stalemate
        else
            return DRAWSCORE;
    }

    
    // found better move
    if (old_alpha != alpha)
    {
        bestmove = best_sofar;
    }

    
    // node (move) fails low
    return alpha;
}



// search
//
// The search is started when the program receives the UCI 'go'
// command. It searches from the root position and outputs the "bestmove".
void search()
{
    // find best move within a given position
    int score = negamax(-INFINITY, INFINITY, Limits.depth);

    
    if (bestmove)
    {
        cout << "info score cp " << score << " depth " << Limits.depth
             << " nodes " <<  nodes << endl << flush;

        cout << "bestmove " << prettyMove(bestmove) << endl << flush;
    }
}



// qsearch
//
// Called by the main search, this is a function with zero depth. qsearch()
// performs an indefinite search until all the following conditions are met:
//
// a) no more possible captures
// b) no more pawn promotions
// c) depth is too deep or time (from a running timer) is up
int qsearch(int alpha, int beta)
{
    // check the clock and the input status
    //if((nodes & 2047 ) == 0)
		//communicate();


    // increment nodes count
    nodes++;


    // we are too deep, hence there's an overflow of arrays relying on max ply constant
    //if (ply > max_ply - 1)
        // evaluate position
    //    return evaluate();

    // calculate "stanidng pat" to stabilize the quiescent search
    int val = evaluate();

    if (val >= beta)
    {
        // node (position) fails high
        return beta;
    }
    
    // found a better move
    if (val > alpha)
    {
        // PV node (position)
        alpha = val;
    }

    
    // create move list instance
    MoveList_t MoveList;

    
    // generate moves
    generateCapturesAndPromotions(MoveList);

    
    // sort moves
    //sortMoves(MoveList, 0);

    
    // loop over moves within a movelist
    for (int count = 0; count < MoveList.count; count++)
    {
        // preserve board state
        saveBoard();
       

        // increment ply
        ply++;

        
        // increment repetition index & store hash key
        //repetition_index++;
        //repetition_table[repetition_index] = hash_key;

        
        // make sure to make only legal moves
        if (!makeMove(MoveList.moves[count], AllMoves))
        {
            // decrement ply
            ply--;
            
            // decrement repetition index
            //repetition_index--;

            takeBack();
            
            // skip to next move
            continue;
        }


        // score current move
        val = -qsearch(-beta, -alpha);
       

        // decrement ply
        ply--;

        
        // decrement repetition index
        //repetition_index--;


        // take move back
        takeBack();


        // reutrn 0 if time is up
        //if (stopped == 1) return 0;

       
        // found a better move
        if (val > alpha)
        {
            // PV node (position)
            alpha = val;
            
            // fail-hard beta cutoff
            if (val >= beta)
            {
                // node (position) fails high
                return beta;
            }
        }
    }
   

    // node (position) fails low
    return alpha;
}



// dperft
//
// Divide-perft is a perft() wrapper that divides a position into each
// root move and calls perft() for each of them. This is very useful
// to debug possible errors within the move generator for a given root move.
void dperft(int depth)
{
    // reliability checks
    assert(depth > 0);


    // reset nodes count
    nodes = 0ULL;
   

    // create move list instance
    MoveList_t MoveList;

    
    // generate moves
    generateMoves(MoveList);

    
    // init start time
    auto start = chrono::high_resolution_clock::now();
   

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


        // cummulative nodes
        uint64_t cNodes = nodes;


        // call perft driver recursively
        perft(depth - 1);

        
        // old nodes
        uint64_t PrevNodes = nodes - cNodes;

        
        // undo move
        takeBack();


        // print move and nodes under that move
        cout << prettyMove(MoveList.moves[move_count]) << ": " << PrevNodes << endl;
    }


    // stop the timer and measure time elapsed
    auto finish = chrono::high_resolution_clock::now();
    auto ns = chrono::duration_cast<chrono::nanoseconds>(finish-start).count();
    assert(ns);


    // print results
    cout << endl;
    cout << "    Depth: " << depth << endl;
    cout << "    Nodes: " << nodes << endl;
    cout << fixed << setprecision(3);
    cout << "    Time:  " << ns / 1000000.0 << "ms" << endl;
    cout << "   Speed:  " << nodes * 1000000 / ns << " Knps" << endl << endl;
}
