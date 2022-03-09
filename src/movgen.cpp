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
//
// TODO: compare performance vs. inline
void generateMoves()
{
    // define source & target squares
    int fromSq, toSq;

    
    // current piece's Bitboard copy & it's attacks
    Bitboard bb, attacks;

    
    // loop over all the bitboards
    for (int piece = P; piece <= k; piece++)
    {
        // init piece bitboard copy
        bb = bitboards[piece];
        
        // generate white pawns & white king castling moves
        if (sideToMove == White)
        {
            // pick up white pawn bitboards index
            if (piece == P)
            {
                // loop over white pawns within white pawn bitboard
                while (bb)
                {
                    // init source square
                    fromSq = ls1b(bb);
                    
                    // init target square
                    toSq = fromSq - 8;

                    
                    // if the target square is on the borad, generate quite pawn moves
                    if (!(toSq < a8) && !getBit(occupancies[Both], toSq))
                    {
                        // pawn promotion
                        if ((fromSq >= a7) && (fromSq <= h7))
                        {
                            cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "q" << endl;
                            cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "r" << endl;
                            cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "b" << endl;
                            cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "n" << endl;
                        }
                        
                        else
                        {
                            // one square ahead pawn move
                            cout << "pawn push: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << endl;
                            
                            // two squares ahead pawn move
                            if (((fromSq >= a2) && (fromSq <= h2)) && !getBit(occupancies[Both], toSq - 8))
                                cout << "double push: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq - 8] << endl;
                        }
                    }


                    // init pawn attacks bitboard
                    attacks = PawnAttacks[sideToMove][fromSq] & occupancies[Black];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        toSq = ls1b(attacks);
                        
                        // pawn promotion
                        if ((fromSq >= a7) && (fromSq <= h7))
                        {
                            cout << "promo capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "q" << endl;
                            cout << "promo capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "r" << endl;
                            cout << "promo capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "b" << endl;
                            cout << "promo capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "n" << endl;
                        }
                        
                        // one square ahead pawn move
                        else
                        {
                            cout << "pawn capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << endl;
                        }
                        
                        // pop ls1b of the pawn attacks
                        popBit(attacks, toSq);
                    }
                    
                    // generate enpassant captures
                    if (epsq != NoSq)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        Bitboard ep_attacks = PawnAttacks[sideToMove][fromSq] & (1ULL << epsq);
                        
                        // make sure enpassant capture available
                        if (ep_attacks)
                        {
                            // init enpassant capture target square
                            int toEp = ls1b(ep_attacks);
                            cout << "ep captpure: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toEp] << endl;
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    popBit(bb, fromSq);
                }
            }


            // castling moves
            if (piece == K)
            {
                // king side castling is available
                if (castle & wk)
                {
                    // make sure square between king and king's rook are empty
                    if (!getBit(occupancies[Both], f1) && !getBit(occupancies[Both], g1))
                    {
                        // make sure king and the f1 squares are not under attacks
                        if (!isSquareAttacked(e1, Black) && !isSquareAttacked(f1, Black))
                            printf("castling move: e1g1\n");
                    }
                }
                
                // queen side castling is available
                if (castle & wq)
                {
                    // make sure square between king and queen's rook are empty
                    if (!getBit(occupancies[Both], d1) && !getBit(occupancies[Both], c1) && !getBit(occupancies[Both], b1))
                    {
                        // make sure king and the d1 squares are not under attacks
                        if (!isSquareAttacked(e1, Black) && !isSquareAttacked(d1, Black))
                            printf("castling move: e1c1\n");
                    }
                }
            }
        }
        
        // generate black pawns & black king castling moves
        else
        {
            // pick up black pawn bitboards index
            if (piece == p)
            {
                // loop over white pawns within white pawn bitboard
                while (bb)
                {
                    // init source square
                    fromSq = ls1b(bb);
                    
                    // init target square
                    toSq = fromSq + 8;

                    
                    // if the target square is on the borad, generate quite pawn moves
                    if (!(toSq > h1) && !getBit(occupancies[Both], toSq))
                    {
                        // pawn promotion
                        if ((fromSq >= a2) && (fromSq <= h2))
                        {
                            cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "q" << endl;
                            cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "r" << endl;
                            cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "b" << endl;
                            cout << "promo: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "n" << endl;
                        }
                        
                        else
                        {
                            // one square ahead pawn move
                            cout << "pawn push: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << endl;
                            
                            // two squares ahead pawn move
                            if (((fromSq >= a7) && (fromSq <= h7)) && !getBit(occupancies[Both], toSq + 8))
                                cout << "double push: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq + 8] << endl;
                        }
                    }


                    // init pawn attacks bitboard
                    attacks = PawnAttacks[sideToMove][fromSq] & occupancies[White];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        toSq = ls1b(attacks);
                        
                        // pawn promotion
                        if ((fromSq >= a2) && (fromSq <= h2))
                        {
                            cout << "promo capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "q" << endl;
                            cout << "promo capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "r" << endl;
                            cout << "promo capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "b" << endl;
                            cout << "promo capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << "n" << endl;
                        }
                        
                        // one square ahead pawn move
                        else
                        {
                            cout << "pawn capture: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toSq] << endl;
                        }
                        
                        // pop ls1b of the pawn attacks
                        popBit(attacks, toSq);
                    }
                    
                    // generate enpassant captures
                    if (epsq != NoSq)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        Bitboard ep_attacks = PawnAttacks[sideToMove][fromSq] & (1ULL << epsq);
                        
                        // make sure enpassant capture available
                        if (ep_attacks)
                        {
                            // init enpassant capture target square
                            int toEp = ls1b(ep_attacks);
                            cout << "ep captpure: " << SquareToCoordinates[fromSq] << SquareToCoordinates[toEp] << endl;
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    popBit(bb, fromSq);
                }
            }


            // castling moves
            if (piece == k)
            {
                // king side castling is available
                if (castle & bk)
                {
                    // make sure square between king and king's rook are empty
                    if (!getBit(occupancies[Both], f8) && !getBit(occupancies[Both], g8))
                    {
                        // make sure king and the f8 squares are not under attacks
                        if (!isSquareAttacked(e8, White) && !isSquareAttacked(f8, White))
                            printf("castling move: e8g8\n");
                    }
                }
                
                // queen side castling is available
                if (castle & bq)
                {
                    // make sure square between king and queen's rook are empty
                    if (!getBit(occupancies[Both], d8) && !getBit(occupancies[Both], c8) && !getBit(occupancies[Both], b8))
                    {
                        // make sure king and the d8 squares are not under attacks
                        if (!isSquareAttacked(e8, White) && !isSquareAttacked(d8, White))
                            printf("castling move: e8c8\n");
                    }
                }
            }
        }
        
        // genarate knight moves
        
        // generate bishop moves
        
        // generate rook moves
        
        // generate queen moves
        
        // generate king moves
    }
}



// generateMoves
//
// Generate all pseudo-legal moves for the current position.
//
// TODO: compare performance vs. inline
void generateMoves2()
{
    int fromSq, toSq;
    Bitboard bb, attacks;
    Bitboard freeSquares = ~occupancies[Both];

    // white pieces
    if (sideToMove == White)
    {
        // white pawns
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


        // white King
        bb = bitboards[K];
        while (bb)
        {
            fromSq = ls1b(bb);
           
            // TODO: king Attacks

            // short castle 0-0
            if (castle & wk) 
            {
                if (!(FG1_Mask & occupancies[Both]))
                {
                    if (!isSquareAttacked(e1, Black) &&
                        !isSquareAttacked(f1, Black) &&
                        !isSquareAttacked(g1, Black))
                    {
                        cout << "castling move: e1g1" << endl;
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
                        cout << "castling move: e1c1" << endl;
                    }
                }
            }

            // move on to next piece (pop ls1b)
            popBit(bb, fromSq);
        }
    }


    // black pieces
    else
    {
        // black pawns
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
        bb = bitboards[K];
        while (bb)
        {
            fromSq = ls1b(bb);
           
            // TODO: king Attacks

            // short castle 0-0
            if (castle & bk) 
            {
                if (!(FG8_Mask & occupancies[Both]))
                {
                    if (!isSquareAttacked(e8, White) &&
                        !isSquareAttacked(f8, White) &&
                        !isSquareAttacked(g8, White))
                    {
                        cout << "castling move: e8g8" << endl;
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
                        cout << "castling move: e8c8" << endl;
                    }
                }
            }

            // move on to next piece (pop ls1b)
            popBit(bb, fromSq);
        }
    }
}
