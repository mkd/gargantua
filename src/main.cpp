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


    // set up position
    setPosition(FENPOS_KIWIPETE);
    printBoard();
   

    // create move list instance
    MoveList_t move_list;
    
    // generate moves
    generateMoves(move_list);

    // loop over generated moves
    cout << "Looping 999999 times over " << move_list.count << " moves..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < 999999; i++)
    {
        for (int move_count = 0; move_count < move_list.count; move_count++)
        {
            // init move
            int move = move_list.moves[move_count];
            
            // preserve board state
            saveBoard();
            
            // make move
            makeMove(move, AllMoves);
            //printBoard();
            //getchar();
            
            // take back
            takeBack();
            //printBoard();
            //getchar();
        }
    }
    auto finish = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::milliseconds>(finish-start).count() << "ms\n";
    
    // add move
    /*
    auto start = chrono::high_resolution_clock::now();
    auto finish = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::nanoseconds>(finish-start).count() << "ns\n";
    */


    // terminate program
    return 0;
}
