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

#include "eval.h"
#include "position.h"
#include "nnue.h"



// evaluate
//
// This evaluation function gives an absolute value with the current position
// evaluation, taking into account both material and positional play. 
//
// The value returned by eval already contains a negative sign (-) for a
// winning score for Black, whereas a positive score means White is better.
//
// Note: this evaluation function solely relies on a neural network (NNUE file)
// that has been trained with hundreds of millions of positions at moderate
// depth using Stockfish.
int evaluate()
{    
    // This function ends up calling the nnue_evaluate() function:
    //
    // nnue_evaluate() takes three arguments:
    //
    // 1. (int)   side to move -- white=0, black=1
    // 2. (int *) array of pieces
    // 3. (int *)  array of squares each piece stands on
    //
    //
    // Piece codes are:
    //      wking=1, wqueen=2, wrook=3, wbishop= 4, wknight= 5, wpawn= 6,
    //      bking=7, bqueen=8, brook=9, bbishop=10, bknight=11, bpawn=12,
    //
    // Squares are:
    //      A1=0, B1=1 ... H8=63
    //
    // Input format:
    //      piece[0] is white king, square[0] is its location
    //      piece[1] is black king, square[1] is its location
    //      ..
    //      piece[x], square[x] can be in any order
    //      ..
    //      piece[n+1] is set to 0 to represent end of array
    //
    // Returns the score relative to side to move in approximate centi-pawns.


    // current pieces bitboard copy
    Bitboard bb;


    // init piece & square
    int piece, square;


    // array of piece codes converted to Stockfish piece codes
    int pieces[33];
   

    // array of square indices converted to Stockfish square indices
    int squares[33];
   

    // pieces and squares current index to write next piece square pair at
    int index = 2;
   

    // loop over piece bitboards
    for (int bb_piece = P; bb_piece <= k; bb_piece++)
    {
        // init piece bitboard copy
        bb = bitboards[bb_piece];
        
        // loop over pieces within a bitboard
        while (bb)
        {
            // init piece
            piece = bb_piece;
            
            // init square
            square = popLsb(bb);
           

            // initialize pieces and squares arrays for NNUE:

            // white king
            if (piece == K)
            {
                pieces[0] = nnue_pieces[piece];
                squares[0] = nnue_squares[square];
            }
            
            // black king
            else if (piece == k)
            {
                pieces[1] = nnue_pieces[piece];
                squares[1] = nnue_squares[square];
            }
            
            // rest of the pieces
            else
            {
                pieces[index] = nnue_pieces[piece];
                squares[index] = nnue_squares[square];
                index++;    
            }
        }
    }
   

    // set zero terminating characters at the end of pieces & squares arrays
    pieces[index] = 0;
    squares[index] = 0;
    
    
    // We need to make sure that fifty rule move counter gives a penalty
    // to the evaluation, otherwise it won't be capable of mating in
    // simple endgames like KQK or KRK! This expression is used:
    //                    nnue_score * (100 - fifty) / 100

    //return (nnue_evaluate(sideToMove, pieces, squares) * (100 - fifty) / 100);
    return (nnue_evaluate(sideToMove, pieces, squares));
}
