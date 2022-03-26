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

#include "nnue.h"
#include "bitboard.h"
#include "position.h"
#include "search.h"
#include "uci.h"



// The program's main application consists of two parts:
// 1) inintialization of the necessary data structures
// 2) UCI loop: interpreting commands from the user input and running them
//
// UCI protocol specs: http://wbec-ridderkerk.nl/html/UCIProtocol.html
int main(int argc, char *argv[])
{
    // print engine info
    cout << ENGINE_NAME << " " << ENGINE_VERSION <<
            " by " << ENGINE_AUTHOR << endl << flush;


    // initializations
    initBitboards();
    initSearch();


    // initialize neural network (NNUE) for evaluation
    nnue_init("nn-eba324f53044.nnue");
    cout << endl << flush;


    // test
    //setPosition("r3k2r/pPppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    //MoveList_t MoveList;
    //generateMoves(MoveList);
    //auto start = chrono::high_resolution_clock::now();
    //for (int i = 0; i < 999999; i++)
    //    sortMoves(MoveList);
    //auto finish = chrono::high_resolution_clock::now();
    //auto ns = chrono::duration_cast<chrono::nanoseconds>(finish-start).count();
    //cout << "Time: " << ns / 1000000.0f << "ms." << endl;
    

    // enter UCI loop
    UCI::loop(argc, argv);


    // terminate program
    return 0;
}
