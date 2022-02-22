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
#include "board.h"
#include "bit.h"


using namespace std;



// displayBitboard
//
// Take a Bitboard and display it as a board representation on the standard
// output.
void displayBitboard(Bitboard bb)
{
    cout << endl;

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // convert file & rank into square index
            int square = rank * 8 + file;

            // print ranks
            if (!file)
                cout << " " << 8 - rank << "  ";

            // print bit state (either 1 or 0)
            cout << " " << (getBit(bb, square) ? "1" : ".");
        }

        // print new line every rank
        cout << endl;
    }

    // print board files
    cout << endl << "     a b c d e f g h" << endl << endl;

    // print bitboard as unsigned decimal number
    cout << "     Bitboard: " << bb << endl << endl;
}
