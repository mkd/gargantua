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

    // black pawns
    if (sideToMove == Black)
    {
        bb = bitboards[p];

        // loop over black pawns
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
                attacks ^= SqBB[toSq];
            }

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
            bb ^= SqBB[fromSq];
        }
    }


    // white pawns
    else
    {
        bb = bitboards[P];

        // loop over white pawns
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
                attacks ^= SqBB[toSq];
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
            bb ^= SqBB[fromSq];
        }
    }
}
