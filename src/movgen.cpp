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

#include "bitboard.h"
#include "position.h"
#include "movgen.h"


using namespace std;



// generateMoves
//
// Generate all pseudo-legal moves for the current position.
void generateMoves()
{
    // Bitboard data and squares to generate moves
    int fromSq, toSq;
    Bitboard bb, attacks;
    Bitboard freeSquares = ~occupancies[Both];


    // Generate moves for White Pieces
    // sideToMove == White
    //
    if (sideToMove == White)
    {
        // White Pawns
        // bitboards[P]
        //
        bb = bitboards[P];
        while (bb)
        {
            fromSq = ls1b(bb);
            attacks = PawnPushes[P][fromSq] & freeSquares;
            if (attacks)
                attacks |= PawnDoublePushes[P][fromSq] & freeSquares;
            attacks |= PawnAttacks[White][fromSq] & occupancies[Black];

            while (attacks)
            {
                toSq = ls1b(attacks);

                // promotions
                if (SqBB[toSq] & Rank8_Mask)
                {
                    cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "q" << endl;
                    cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "r" << endl;
                    cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "b" << endl;
                    cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "n" << endl;
                }

                // pawn pushes and attacks
                else
                {
                    cout << "pmove: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << endl;
                }

                // pop ls1b
                popBit(attacks, toSq);
            }

            // enpassant capture
            if (epsq)
            {
                if (PawnAttacks[White][fromSq] & SqBB[epsq])
                {
                    if (bitboards[p] & SqBB[epsq + 8])
                    {
                        toSq = epsq;
                        cout << "enpas: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << endl;
                    }
                }
            }

            // pop ls1b
            popBit(bb, fromSq);
        }


        // White King
        // bitboards[K]
        //
        bb = bitboards[K];
        while (bb)
        {
            fromSq = ls1b(bb);
           
            // init piece attacks in order to get set of target squares
            attacks = KingAttacks[fromSq] & ~occupancies[White];
                
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[Black], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  king quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  king capture" << endl;
                    
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }


            // short castle 0-0
            if (castle & wk) 
            {
                if (!(FG1_Mask & occupancies[Both]))
                {
                    if (!isSquareAttacked(e1, Black) &&
                        !isSquareAttacked(f1, Black) &&
                        !isSquareAttacked(g1, Black))
                    {
                        cout << "e1g1  castling move" << endl;
                    }
                }
            }

            // long castle 0-0-0
            if (castle & wq)
            {
                if (!(DCB1_Mask & occupancies[Both]))
                {
                    if (!isSquareAttacked(e1, Black) &&
                        !isSquareAttacked(d1, Black) &&
                        !isSquareAttacked(c1, Black))
                    {
                        cout << "e1c1  castling move" << endl;
                    }
                }
            }

            // move on to next piece (pop ls1b)
            popBit(bb, fromSq);
        }

        // White Knights
        // bitboards[N]
        bb = bitboards[N];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = KnightAttacks[fromSq] & ~occupancies[White];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[Black], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  knight quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  knight capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }
            
            
            // pop ls1b of the current piece bitboard copy
            popBit(bb, fromSq);
        }


        // White Bishops
        // bitboards[B]
        bb = bitboards[B];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = getBishopAttacks(fromSq, occupancies[Both]) & ~occupancies[White];

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[Black], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  bishop quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  bishop capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }
            
            
            // pop ls1b of the current piece bitboard copy
            popBit(bb, fromSq);
        }


        // White Rooks
        // bitboards[R]
        //
        bb = bitboards[R];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = getRookAttacks(fromSq, occupancies[Both]) & ~occupancies[White];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[Black], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  rook quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  rook capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }
            
            
            // pop ls1b of the current piece bitboard copy
            popBit(bb, fromSq);
        }



        // White Queens
        // bitboards[Q]
        //
        bb = bitboards[Q];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = getQueenAttacks(fromSq, occupancies[Both]) & ~occupancies[White];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[Black], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  queen quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  queen capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }
            
            
            // pop ls1b of the current piece bitboard copy
            popBit(bb, fromSq);
        }
    }


    // Generate moves for Black Pieces
    // sideToMove == Black
    //
    else
    {
        // Black Pawns
        // bitboard[p]
        //
        bb = bitboards[p];
        while (bb)
        {
            fromSq = ls1b(bb);
            attacks = PawnPushes[Black][fromSq] & freeSquares;
            if (attacks)
                attacks |= PawnDoublePushes[Black][fromSq] & freeSquares;
            attacks |= PawnAttacks[Black][fromSq] & occupancies[White];

            while (attacks)
            {
                toSq = ls1b(attacks);

                // promotions
                if (SqBB[toSq] & Rank1_Mask)
                {
                    cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "q" << endl;
                    cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "r" << endl;
                    cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "b" << endl;
                    cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "n" << endl;
                }
                else
                {
                    cout << "pmove: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << endl;
                }

                // pop ls1b
                popBit(attacks, toSq);
            }

            // enpassant capture
            if (epsq)
            {
                if (PawnAttacks[Black][fromSq] & SqBB[epsq])
                {
                    if (bitboards[P] & SqBB[epsq - 8])
                    {
                        toSq = epsq;
                        cout << "enpas: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << endl;
                    }
                }
            }

            // pop ls1b
            popBit(bb, fromSq);
        }


        // white King
        bb = bitboards[k];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = KingAttacks[fromSq] & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[White], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  king quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  king capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }


            // short castle 0-0
            if (castle & bk) 
            {
                if (!(FG8_Mask & occupancies[Both]))
                {
                    if (!isSquareAttacked(e8, White) &&
                        !isSquareAttacked(f8, White) &&
                        !isSquareAttacked(g8, White))
                    {
                        cout << "e8g8  castling move" << endl;
                    }
                }
            }

            // long castle 0-0-0
            if (castle & bq)
            {
                if (!(DCB8_Mask & occupancies[Both]))
                {
                    if (!isSquareAttacked(e8, White) &&
                        !isSquareAttacked(d8, White) &&
                        !isSquareAttacked(c8, White))
                    {
                        cout << "e8c8  castling move" << endl;
                    }
                }
            }

            // move on to next piece (pop ls1b)
            popBit(bb, fromSq);
        }


        // Black Knights
        // bitboards[n]
        //
        bb = bitboards[n];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = KnightAttacks[fromSq] & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[White], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  knight quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  knight capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }
            
            
            // pop ls1b of the current piece bitboard copy
            popBit(bb, fromSq);
        }


        // Black Bishops
        // bitboards[b]
        //
        bb = bitboards[b];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = getBishopAttacks(fromSq, occupancies[Both]) & ~occupancies[Black];

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[White], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  bishop quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  bishop capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }
            
            
            // pop ls1b of the current piece bitboard copy
            popBit(bb, fromSq);
        }


        // Black Rooks
        // bitboards[r]
        //
        bb = bitboards[r];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = getRookAttacks(fromSq, occupancies[Both]) & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[White], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  rook quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  rook capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }
            
            
            // pop ls1b of the current piece bitboard copy
            popBit(bb, fromSq);
        }


        // Black Queens
        // bitboards[q]
        //
        bb = bitboards[q];
        while (bb)
        {
            // init source square
            fromSq = ls1b(bb);
            
            // init piece attacks in order to get set of target squares
            attacks = getQueenAttacks(fromSq, occupancies[Both]) & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = ls1b(attacks);    
                
                // quite move
                if (!getBit(occupancies[White], toSq))
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  queen quiet move" << endl;
                
                else
                    // capture move
                    cout << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "  queen capture" << endl;
                
                // pop ls1b in current attacks set
                popBit(attacks, toSq);
            }
            
            
            // pop ls1b of the current piece bitboard copy
            popBit(bb, fromSq);
        }
    }
}
