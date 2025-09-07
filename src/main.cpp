/*
  This file is part of Gargantua, a UCI chess engine with NNUE evaluation
  derived from Chess0, and inspired by Code Monkey King's bbc-1.4.
     
  Copyright (C) 2025 Claudio M. Camacho
 
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
#include "tt.h"
#include "tbprobe.h"



// The program's main application consists of two parts:
// 1) inintialization of the necessary data structures
// 2) UCI loop: interpreting commands from the user input and running them
//    on the engine
//
// UCI protocol specs: http://wbec-ridderkerk.nl/html/UCIProtocol.html
int main(int argc, char *argv[])
{
    // print engine info
    cout << EngineName << " " << EngineVersion << " by "
         << EngineAuthor << endl << flush;


    // initializations
    initBitboards();
    initRandomKeys();
    initSearch();


    // initialize hash table (cache)
    TT::init(1024);


    // initialize tablebases (syzygy)
    // if (options["SyzygyPath"] != "")
    //     tb_init(options["SyzygyPath"])   
    // SyzygyPath = syzygy -- by default


    // initialize neural network (NNUE) for evaluation
    nnue_init("nn-eba324f53044.nnue");


    // enter UCI loop
    cout << endl << flush;
    UCI::loop(argc, argv);


    // terminate program
    return 0;
}
