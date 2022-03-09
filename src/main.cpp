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

#include <chrono>

#include "nnue.h"
#include "bitboard.h"
#include "position.h"
#include "movgen.h"


using namespace std;



// The program's main application consists of two parts:
// 1) inintialization of the necessary data structures
// 2) UCI loop: interpreting commands from the user input and running them
//
// UCI protocol specs: http://wbec-ridderkerk.nl/html/UCIProtocol.html
int main(void)
{
    // Initialize neural network
    nnue_init("nn-eba324f53044.nnue");
    std::cout << std::endl;


    // init piece moves and data structures
    initBitmaps();
    initLeaperAttacks();
    initSliderAttacks(Bishop);
    initSliderAttacks(Rook);

    setPosition("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ");
    printBoard();

    auto start = chrono::high_resolution_clock::now();
    generateMoves();
    auto finish = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::nanoseconds>(finish-start).count() << "ns\n";

    cout << endl << endl;

    auto start1 = chrono::high_resolution_clock::now();
    generateMoves2();
    auto finish1 = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::nanoseconds>(finish1-start1).count() << "ns\n";



   
    // terminate program
    return 0;
}
