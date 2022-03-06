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
#include "io.h"



// The program's main application consists of two parts:
// 1) inintialization of the necessary data structures
// 2) UCI loop: interpreting commands from the user input and running them
//
// UCI protocol specs: http://wbec-ridderkerk.nl/html/UCIProtocol.html
int main(void)
{
    // Initialize neural network
    //nnue_init("nn-eba324f53044.nnue");
    std::cout << std::endl;


    // init piece moves and data structures
    initLeaperAttacks();
    initSliderAttacks(Bishop);
    initSliderAttacks(Rook);



    // test
    setPosition("r1bq1bnr/pp2pkpp/2nP4/2p5/P4pP1/R4N2/1PPP1P1P/1NBQKB1R b K g3");
    printBoard();
    setPosition("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");
    printBoard();

   
    // terminate program
    return 0;
}
