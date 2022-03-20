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
    cout << endl;


    // init piece moves and data structures
    initBitmaps();
    initLeaperAttacks();
    initSliderAttacks(Bishop);
    initSliderAttacks(Rook);

    

    setPosition(FENPOS_STARTPOS);
    printBoard();
    StateInfo si;
    si.previous = st;
    st = &si;
    dperft(4);
    return 0;


/*
    MoveList_t moves;
    generateMoves(moves);

    for (int i = 0; i < moves.count; i++)
    {
        // save board state
        StateInfo nsi = *st;
        nsi.id = st->id + 1;
        nsi.previous = st;
        st = &nsi;
        cout << "Saving board state; stID = " << st->id << endl;

        // make move
        if (!makeMove(moves.moves[i], AllMoves))
        {
            undoMove(moves.moves[i]);
            continue;
        }

        undoMove(moves.moves[i]);
        cout << "Restored board state; stID = " << st->id << endl;
    }
    return 0;
    */


    // test perft on different positions
    cout << "Test 'startpos': " << endl;
    setPosition(FENPOS_STARTPOS);
    dperft(6);

    cout << "Test 'kiwipete': " << endl;
    setPosition(FENPOS_KIWIPETE);
    dperft(6);

    cout << "Test 'pos3': " << endl;
    setPosition(FENPOS_POS3);
    dperft(6);

    cout << "Test 'pos4': " << endl;
    setPosition(FENPOS_POS4);
    dperft(6);

    cout << "Test 'pos5': " << endl;
    setPosition(FENPOS_POS5);
    dperft(6);

    cout << "Test 'pos6': " << endl;
    setPosition(FENPOS_POS6);
    dperft(6);



    // terminate program
    return 0;
}
