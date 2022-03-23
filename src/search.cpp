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



// 'nodes' is a global variable holding the number of nodes analyzed
// or searched. It is used by negamax() but also other performance test
// functions such as perft().
uint64_t nodes = 0ULL;



// Limits holds the configuration of the search: time, search depth, etc.
Limits_t Limits;



// dperft
//
// Divide-perft is a perft() wrapper that divides a position into each
// root move and calls perft() for each of them. This is very useful
// to debug possible errors within the move generator for a given root move.
void dperft(int depth)
{
    // reliability checks
    assert(depth < 0);


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
        //StateInfo newState = *st;
        //newState.previous = st;
        //st = &newState;


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
    Limits.depth = 0;
    Limits.mate = 0;
    Limits.perft = 0;
    Limits.infinite = 0;
    Limits.nodes = 0;
}



// search
//
// The search is started when the program receives the UCI 'go'
// command. It searches from the root position and outputs the "bestmove".
void search()
{
    // test
    std::cout << "Limits.wtime = "      << Limits.wtime     << std::endl;
    std::cout << "Limits.btime = "      << Limits.btime     << std::endl;
    std::cout << "Limits.winc = "       << Limits.winc      << std::endl;
    std::cout << "Limits.binc = "       << Limits.binc      << std::endl;
    std::cout << "Limits.npmsec = "     << Limits.npmsec    << std::endl;
    std::cout << "Limits.movetime = "   << Limits.movetime  << std::endl;
    std::cout << "Limits.movestogo = "  << Limits.movestogo << std::endl;
    std::cout << "Limits.depth = "      << Limits.depth     << std::endl;
    std::cout << "Limits.mate = "       << Limits.mate      << std::endl;
    std::cout << "Limits.perft = "      << Limits.perft     << std::endl;
    std::cout << "Limits.infinite = "   << Limits.infinite  << std::endl;
    std::cout << "Limits.nodes = "      << Limits.nodes     << std::endl;
}
