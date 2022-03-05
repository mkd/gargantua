/*
  This file is part of Gargantua, a UCI chess engine derived from Chess0 which
  uses NNUE for evaluation. This engine has been inspired by BBC-1.4 (by Code
  Monkey King), Blunder (by Christian Dean) and Stockfish.
     
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
#include <bit>
#include <bitset>
#include "bitboard.h"
#include "nnue.h"





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

    
    // define test bitboard
    Bitboard occupancy = 0ULL;
    
    // set blocker pieces on board
    setBit(occupancy, c5);
    setBit(occupancy, f2);
    setBit(occupancy, g7);
    setBit(occupancy, b2);
    setBit(occupancy, g5);
    setBit(occupancy, e2);
    setBit(occupancy, e7);
    Bitboard b = DarkSquares;

    
    // print rook attacks
    printBitboard(b);
    int j = 0;
    for (uint64_t i = 0; i < 999999999; i++)
    {
        j = countBits(b);
    }
    std::cout << "count = " << j << std::endl;
    printBitboard(b);

   
    // terminate program
    return 0;
}
