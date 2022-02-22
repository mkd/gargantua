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
#include "bit.h"
#include "board.h"
#include "nnue.h"


using namespace std;



int main(void)
{
    // Initialize the neural network weights
    nnue_init("nn-eba324f53044.nnue");

    // define bitboard
    Bitboard b = 0ULL;
    displayBitboard(b);

    // setting some bits
    setBit(b, e4);
    toggleBit(b, c3);
    displayBitboard(b);

    // print bitboard
    toggleBit(b, h1);
    displayBitboard(b);

    // print bitboard
    clearBit(b, c3);
    displayBitboard(b);

    cout << "h1 = " << getBit(b, h1) << endl;

    return 0;
}
