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

   
    // set white pawns
    setBit(bitboards[P], a2);
    setBit(bitboards[P], b2);
    setBit(bitboards[P], c2);
    setBit(bitboards[P], d2);
    setBit(bitboards[P], e2);
    setBit(bitboards[P], f2);
    setBit(bitboards[P], g2);
    setBit(bitboards[P], h2);
    
    // set white knights
    setBit(bitboards[N], b1);
    setBit(bitboards[N], g1);
    
    // set white bishops
    setBit(bitboards[B], c1);
    setBit(bitboards[B], f1);
    
    // set white rooks
    setBit(bitboards[R], a1);
    setBit(bitboards[R], h1);
    
    // set white queen & king
    setBit(bitboards[Q], d1);
    setBit(bitboards[K], e1);
    
    // set white pawns
    setBit(bitboards[p], a7);
    setBit(bitboards[p], b7);
    setBit(bitboards[p], c7);
    setBit(bitboards[p], d7);
    setBit(bitboards[p], e7);
    setBit(bitboards[p], f7);
    setBit(bitboards[p], g7);
    setBit(bitboards[p], h7);
    
    // set white knights
    setBit(bitboards[n], b8);
    setBit(bitboards[n], g8);
    
    // set white bishops
    setBit(bitboards[b], c8);
    setBit(bitboards[b], f8);
    
    // set white rooks
    setBit(bitboards[r], a8);
    setBit(bitboards[r], h8);
    
    // set white queen & king
    setBit(bitboards[q], d8);
    setBit(bitboards[k], e8);
    
    // init side
    sideToMove = Black;
    
    // init enpassant
    epsq = e3;
    
    // init castling
    castle |= wk;
    castle |= wq;
    castle |= bk;
    castle |= bq;
    
    // print chess board
    printBoard();
   
    // terminate program
    return 0;
}
