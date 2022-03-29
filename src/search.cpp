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



// killers [id][ply] 
//
// Killers is a table where the two best (quiet) moves are
// systematically stored for later searches. This is based on the
// fact that a move producing a beta cut-off must be a good one.
// beta cut-offs, where a move killer moves [id][ply]
//
// Note: storing exactly 2 killer moves is best for efficiency/performance.
int killers[2][MAXPLY];



// history [piece][square]
//
// History is a table where to store moves that have produced an improvement in
// the score of previous searches. In other words, they have raised alpha.
int history[12][64];



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


    // increment nodes count
    nodes++;


    // is king in check?
    bool inCheck = isSquareAttacked((sideToMove == White) ? ls1b(bitboards[K]) : 
                                                            ls1b(bitboards[k]),
                                                            sideToMove ^ 1);


    // Extend the search depth by one if we're in check, so that we're less
    // likely to make a tactical mistake. I.e., don't call quiescence search
    // while in check.
    if (inCheck)
        depth++;


    // leaf node: return static evaluation
    if (depth == 0)
        return qsearch(alpha, beta);


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


    // sort moves from best to worst
    sortMoves(MoveList);

    
    // loop over moves within a movelist
    for (int count = 0; count < MoveList.count; count++)
    {
        // preserve board state
        saveBoard();
       

        // increment ply
        ply++;
       

        // make sure to make only legal moves
        if (!makeMove(MoveList.moves[count]))
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
            // store killer moves
            killers[1][ply] = killers[0][ply];
            killers[0][ply] = MoveList.moves[count];


            // node (move) fails high
            return beta;
        }

        
        // found a better move (improves alpha)
        if (score > alpha)
        {
            // store history moves
            history[getMovePiece(MoveList.moves[count])][getMoveTarget(MoveList.moves[count])] += depth;


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
    // reliability checks
    assert(Limits.depth >= 0);


    // start the timer as soon as possible
    auto start = chrono::high_resolution_clock::now();


    // reset data structures for a new search
    memset(killers, 0, sizeof(killers));
    memset(history, 0, sizeof(history));


    // define initial alpha beta bounds
    int alpha = -VALUE_INFINITE;
    int beta  =  VALUE_INFINITE;


    // reset nodes counter
    nodes = 0;


    // find best move within a given position
    int score = negamax(alpha, beta, Limits.depth);


    // stop the timer and measure time elapsed
    auto finish = chrono::high_resolution_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(finish-start).count();
    auto ns = chrono::duration_cast<chrono::nanoseconds>(finish-start).count();


    // print best move, if any
    if (bestmove)
    {
        cout << "info depth " << Limits.depth << " score cp " << score
             << " nodes " <<  nodes << " nps " << nodes * 1000000000 / ns
             << " time " << ms << endl << flush;

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
    // start searching a score from the beginning (= -VALUE_INFINITE)
    int val, score;


    // check the clock and the input status
    //if((nodes & 2047 ) == 0)
		//communicate();


    // increment nodes count
    nodes++;


    // we are too deep, hence there's an overflow of arrays relying on max ply constant
    if (ply > MAXPLY - 1)
        return evaluate();


    // check for an immediate draw
    //if (isDraw())
    // return DRAWSCORE;
    // is king in check?
    //bool inCheck = isSquareAttacked((sideToMove == White) ? ls1b(bitboards[K]) : 
    //                                                        ls1b(bitboards[k]),
    //                                                        sideToMove ^ 1);


    // Extend the search depth by one if we're in check, so that we're less
    // likely to make a tactical mistake. I.e., don't call quiescence search
    // while in check.
    //if (inCheck)
    //    return negamax(alpha, beta, 1);


    // calculate "stand-pat" to stabilize the qsearch
    val = evaluate();


    // beta-cutoff
    if (val >= beta)
        return beta;

    
    // found a better move (PV node position)
    if (val > alpha)
        alpha = val;
   

    // This check extension must be commented out, if it already exists within
    // negamax(). Otherwise, it creates an infinite loop and the program
    // crashes:
    //else
    //    return negamax(alpha, beta, 1);

    
    // create move list instance
    MoveList_t MoveList;

    
    // generate moves
    generateCapturesAndPromotions(MoveList);

    
    // sort moves
    sortMoves(MoveList);

    
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
        if (!makeMove(MoveList.moves[count]))
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
        score = -qsearch(-beta, -alpha);
       

        // decrement ply
        ply--;

        
        // decrement repetition index
        //repetition_index--;


        // take move back
        takeBack();


        // reutrn 0 if time is up
        //if (stopped == 1) return 0;

       
        // found a better move
        if (score > alpha)
        {
            // PV node (position)
            alpha = score;
            
            // fail-hard beta cutoff
            if (score >= beta)
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
        if (!makeMove(MoveList.moves[move_count]))
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



// sortMoves
//
// Take a MoveList_t and sort it in descending order by move score.
//
// sortMoves() works in a way that goes through every move in the move
// list, gets a score for every move (based on MVV/LVA, PV node, and
// other heuristics), and sorts the list from bigger score to smaller 
// score. In other words, sortMoves() tries to bring the best move to
// the front.
void sortMoves(MoveList_t &MoveList)
{
    // reliability checks
    assert(MoveList.count > 0);
    assert(MoveList.count < 256);


    // associative array linking moves and scores
    pair<int, int> pairt[256];


    // find the score for every move
    for (int i = 0; i < MoveList.count; i++) 
    {
        pairt[i].first  = scoreMove(MoveList.moves[i]);
        pairt[i].second = MoveList.moves[i];
    }
  
    // sort the associative array based on the move score
    sort(pairt, pairt + MoveList.count, greater<pair<int, int>>());
     

    // re-write the MoveList ordered by move score in descending order
    for (int i = 0; i < MoveList.count; i++) 
        MoveList.moves[i] = pairt[i].second;
}



// printMoveScores
//
// This function is for testing move scoring and ordering.
void printMoveScores(MoveList_t &MoveList)
{
    cout << "     Move scores:" << endl << endl;
       

    // loop over moves within a move list
    for (int count = 0; count < MoveList.count; count++)
    {
        cout << "     move: ";
        cout << prettyMove(MoveList.moves[count]);
        cout << " score: " << scoreMove(MoveList.moves[count]) << endl;
    }
    cout << endl << endl;
}
