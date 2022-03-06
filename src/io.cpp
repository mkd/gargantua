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
#include <iomanip>

#include "bitboard.h"
#include "position.h"



using namespace std;



// printBoard
//
// Convert the internal representation of the board into a human-readable string
// (capable of being shown and represented as a Board in ASCII) and show it on
// the screen.
void printBoard()
{
    cout << "    +----+----+----+----+----+----+----+----+" << endl;


    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        cout << setw(3) << 8 - rank <<  " |";


        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // convert file & rank into square index
            int square = rank * 8 + file;

            // loop over all piece bitboards
            int piece = -1;
            Side color = NoColor;
            for (int bb_piece = P; bb_piece <= k; bb_piece++)
            {
                if (getBit(bitboards[bb_piece], square))
                    piece = bb_piece;

                switch (piece)
                {
                    case p: 
                    case n: 
                    case b: 
                    case r: 
                    case q: 
                    case k: color = Black;
                            break;
                    default: color = White;
                }
            }

            // print bit state (either 1 or 0)
            cout << " " << ((piece == -1) ? " " : PieceStr[piece]);
            cout << ((color == White) ? " " : "*") << " |";
        }


        // print new line every rank
        cout << endl << "    +----+----+----+----+----+----+----+----+" << endl;
    }


    // print board files
    cout << "      a    b    c    d    e    f    g    h" << endl << endl;



    // print board status
    cout << "Side on move:   " << ((sideToMove == White) ? "White" : "Black") << endl;
    cout << "Enpassant sq:   " << ((epsq != NoSq) ? SquareToCoordinates[epsq] : "-") << endl;
    cout << "Castling:       " << ((castle & wk) ? "K" : "-") <<
                                  ((castle & wq) ? "Q" : "-") <<
                                  ((castle & bk) ? "k" : "-") <<
                                  ((castle & bq) ? "q" : "-") << endl;
}
