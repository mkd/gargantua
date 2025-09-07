/*
  This file is part of Gargantua, a UCI chess engine with NNUE evaluation
  derived from Chess0, and inspired by Code Monkey King's bbc-1.4.
     
  Copyright (C) 2025 Claudio M. Camacho
 
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
#include <cassert>

#include "bitboard.h"
#include "position.h"
#include "movgen.h"



using namespace std;



// generateMoves
//
// Generate all pseudo-legal moves for the current position.
void generateMoves(MoveList_t &MoveList)
{
    int fromSq, toSq;
    Bitboard attacks = 0ULL;


    // Bitboard containing the pieces for the side on move
    Bitboard Us = occupancies[sideToMove];


    // start with an empty move list
    MoveList.count = 0;


    // iterate over all the pieces from the side on move
    while (Us)
    {
        // get next piece and its location, then clean it from the "Us" Bitboard
        fromSq = popLsb(Us);


        // White Pawns
        if (SqBB[fromSq] & bitboards[P])
        {
            // init target square
            toSq = fromSq - 8;

            // generate quiet pawn moves
            if (!(toSq < a8) && !getBit(occupancies[Both], toSq))
            {
                // pawn promotions
                if (SqBB[toSq] & Rank8_Mask)
                {                            
                    addMove(MoveList, encodeMove(fromSq, toSq, P, Q, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, R, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, B, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, N, 0, 0, 0, 0));
                }
                
                else
                {
                    // one-square pawn push
                    addMove(MoveList, encodeMove(fromSq, toSq, P, 0, 0, 0, 0, 0));
                    
                    // double pawn push
                    if ((SqBB[fromSq] & Rank2_Mask) && !(SqBB[toSq - 8] & occupancies[Both]))
                        addMove(MoveList, encodeMove(fromSq, (toSq - 8), P, 0, 0, 1, 0, 0));
                }
            }
                    
            // init pawn attacks bitboard
            attacks = PawnAttacks[White][fromSq] & occupancies[Black];
            
            // generate pawn captures
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);
                
                // pawn promotions with capture
                if (SqBB[toSq] & Rank8_Mask)
                {
                    addMove(MoveList, encodeMove(fromSq, toSq, P, Q, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, R, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, B, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, N, 1, 0, 0, 0));
                }
                
                else
                    // one-square pawn push
                    addMove(MoveList, encodeMove(fromSq, toSq, P, 0, 1, 0, 0, 0));
            }
                    
            // generate enpassant captures
            if (epsq != NoSq)
            {
                // lookup pawn attacks and bitwise AND with enpassant square (bit)
                Bitboard enpassant_attacks = PawnAttacks[White][fromSq] & (1ULL << epsq);
                        
                // make sure enpassant capture available
                if (enpassant_attacks)
                {
                    // init enpassant capture target square
                    int target_enpassant = ls1b(enpassant_attacks);
                    addMove(MoveList, encodeMove(fromSq, target_enpassant, P, 0, 1, 0, 1, 0));
                }
            }
        }


        // Black Pawns
        else if (SqBB[fromSq] & bitboards[p])
        {
            // init target square
            toSq = fromSq + 8;
            
            // generate quiet pawn moves
            if (!(toSq > h1) && !getBit(occupancies[Both], toSq))
            {
                // pawn promotions
                if (SqBB[toSq] & Rank1_Mask)
                {
                    addMove(MoveList, encodeMove(fromSq, toSq, p, q, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, r, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, b, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, n, 0, 0, 0, 0));
                }
                
                else
                {
                    // one-square pawn push
                    addMove(MoveList, encodeMove(fromSq, toSq, p, 0, 0, 0, 0, 0));
                    
                    // double pawn push
                    if ((SqBB[fromSq] & Rank7_Mask) && !(SqBB[toSq + 8] & occupancies[Both]))
                        addMove(MoveList, encodeMove(fromSq, (toSq + 8), p, 0, 0, 1, 0, 0));
                }
            }
            
            // init pawn attacks bitboard
            attacks = PawnAttacks[Black][fromSq] & occupancies[White];
            
            // generate pawn captures
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);
                
                // pawn promotions with capture
                if (SqBB[toSq] & Rank1_Mask)
                {
                    addMove(MoveList, encodeMove(fromSq, toSq, p, q, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, r, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, b, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, n, 1, 0, 0, 0));
                }
                
                else
                    // one-square ahead pawn move
                    addMove(MoveList, encodeMove(fromSq, toSq, p, 0, 1, 0, 0, 0));
            }

            // generate enpassant captures
            if (epsq != NoSq)
            {
                // lookup pawn attacks and bitwise AND with enpassant square (bit)
                Bitboard enpassant_attacks = PawnAttacks[Black][fromSq] & (1ULL << epsq);
                
                // make sure enpassant capture available
                if (enpassant_attacks)
                {
                    // init enpassant capture target square
                    int target_enpassant = ls1b(enpassant_attacks);
                    addMove(MoveList, encodeMove(fromSq, target_enpassant, p, 0, 1, 0, 1, 0));
                }
            }
        }


        // White Knights
        else if (SqBB[fromSq] & bitboards[N])
        {
            // init piece attacks in order to get set of target squares
            attacks = KnightAttacks[fromSq] & ~occupancies[White];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[Black] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, N, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, N, 0, 1, 0, 0, 0));
            }
        }


        // Black Knights
        else if (SqBB[fromSq] & bitboards[n])
        {
            // init piece attacks in order to get set of target squares
            attacks = KnightAttacks[fromSq] & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[White] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, n, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, n, 0, 1, 0, 0, 0));
            }
        }


        // White Bishops
        else if (SqBB[fromSq] & bitboards[B])
        {
            // init piece attacks in order to get set of target squares
            attacks = getBishopAttacks(fromSq, occupancies[Both]) & ~occupancies[White];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[Black] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, B, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, B, 0, 1, 0, 0, 0));
            }
        }


        // Black Bishops
        else if (SqBB[fromSq] & bitboards[b])
        {
            // init piece attacks in order to get set of target squares
            attacks = getBishopAttacks(fromSq, occupancies[Both]) & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[White] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, b, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, b, 0, 1, 0, 0, 0));
            }
        }


        // White Rooks
        else if (SqBB[fromSq] & bitboards[R])
        {
            // init piece attacks in order to get set of target squares
            attacks = getRookAttacks(fromSq, occupancies[Both]) & ~occupancies[White];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[Black] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, R, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, R, 0, 1, 0, 0, 0));
            }
        }


        // Black Rooks
        else if (SqBB[fromSq] & bitboards[r])
        {
            // init piece attacks in order to get set of target squares
            attacks = getRookAttacks(fromSq, occupancies[Both]) & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[White] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, r, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, r, 0, 1, 0, 0, 0));
            }
        }


        // White Queens
        else if (SqBB[fromSq] & bitboards[Q])
        {
            // init piece attacks in order to get set of target squares
            attacks = getQueenAttacks(fromSq, occupancies[Both]) & ~occupancies[White];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[Black] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, Q, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, Q, 0, 1, 0, 0, 0));
            }
        }


        // Black Queens
        else if (SqBB[fromSq] & bitboards[q])
        {
            // init piece attacks in order to get set of target squares
            attacks = getQueenAttacks(fromSq, occupancies[Both]) & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[White] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, q, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, q, 0, 1, 0, 0, 0));
            }
        }


        // White King
        else if (SqBB[fromSq] & bitboards[K])
        {
            // init piece attacks in order to get set of target squares
            attacks = KingAttacks[fromSq] & ~occupancies[White];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[Black] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, K, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, K, 0, 1, 0, 0, 0));
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
                        addMove(MoveList, encodeMove(e1, g1, K, 0, 0, 0, 0, 1));
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
                        addMove(MoveList, encodeMove(e1, c1, K, 0, 0, 0, 0, 1));
                    }
                }
            }
        }


        // Black King
        else if (SqBB[fromSq] & bitboards[k])
        {
            // init piece attacks in order to get set of target squares
            attacks = KingAttacks[fromSq] & ~occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                toSq = popLsb(attacks);    
                
                // quiet move
                if (!(occupancies[White] & SqBB[toSq]))
                    addMove(MoveList, encodeMove(fromSq, toSq, k, 0, 0, 0, 0, 0));
                
                else
                    // capture move
                    addMove(MoveList, encodeMove(fromSq, toSq, k, 0, 1, 0, 0, 0));
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
                        addMove(MoveList, encodeMove(e8, g8, k, 0, 0, 0, 0, 1));
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
                        addMove(MoveList, encodeMove(e8, c8, k, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
    }
}



// generateCapturesAndPromotions
//
// Generate all pseudo-legal captures and promotions for the current position.
// This is typically used by the quiescence search.
void generateCapturesAndPromotions(MoveList_t &MoveList)
{
    int fromSq, toSq;
    Bitboard captures = 0ULL;


    // Bitboard containing the pieces for the side on move
    Bitboard Us = occupancies[sideToMove];


    // start with an empty move list
    MoveList.count = 0;


    // iterate over all the pieces from the side on move
    while (Us)
    {
        // get next piece and its location, then clean it from the "Us" Bitboard
        fromSq = popLsb(Us);


        // White Pawns
        if (SqBB[fromSq] & bitboards[P])
        {
            // init target square
            toSq = fromSq - 8;

            // generate pawn promotions
            if (!(toSq < a8) && !getBit(occupancies[Both], toSq))
            {
                // pawn promotions
                if (SqBB[toSq] & Rank8_Mask)
                {                            
                    addMove(MoveList, encodeMove(fromSq, toSq, P, Q, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, R, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, B, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, N, 0, 0, 0, 0));
                }
            }
                    
            // init pawn attacks bitboard
            captures = PawnAttacks[White][fromSq] & occupancies[Black];
            
            // generate pawn captures
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);
                
                // pawn promotions with capture
                if (SqBB[toSq] & Rank8_Mask)
                {
                    addMove(MoveList, encodeMove(fromSq, toSq, P, Q, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, R, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, B, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, P, N, 1, 0, 0, 0));
                }
                
                // regular capture (without promotion)
                else
                    addMove(MoveList, encodeMove(fromSq, toSq, P, 0, 1, 0, 0, 0));
            }
                    
            // generate enpassant captures
            if (epsq != NoSq)
            {
                // lookup pawn attacks and bitwise AND with enpassant square (bit)
                Bitboard ep_captures = PawnAttacks[White][fromSq] & (1ULL << epsq);
                        
                // make sure enpassant capture available
                if (ep_captures)
                {
                    // init enpassant capture target square
                    int target_enpassant = ls1b(ep_captures);
                    addMove(MoveList, encodeMove(fromSq, target_enpassant, P, 0, 1, 0, 1, 0));
                }
            }
        }


        // Black Pawns
        else if (SqBB[fromSq] & bitboards[p])
        {
            // init target square
            toSq = fromSq + 8;
            
            // generate pawn promotions
            if (!(toSq > h1) && !getBit(occupancies[Both], toSq))
            {
                // pawn promotions
                if (SqBB[toSq] & Rank1_Mask)
                {
                    addMove(MoveList, encodeMove(fromSq, toSq, p, q, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, r, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, b, 0, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, n, 0, 0, 0, 0));
                }
            }
            
            // init pawn attacks bitboard
            captures = PawnAttacks[Black][fromSq] & occupancies[White];
            
            // generate pawn captures
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);
                
                // pawn promotions with capture
                if (SqBB[toSq] & Rank1_Mask)
                {
                    addMove(MoveList, encodeMove(fromSq, toSq, p, q, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, r, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, b, 1, 0, 0, 0));
                    addMove(MoveList, encodeMove(fromSq, toSq, p, n, 1, 0, 0, 0));
                }
                
                // regular capture (without promotion)
                else
                    addMove(MoveList, encodeMove(fromSq, toSq, p, 0, 1, 0, 0, 0));
            }

            // generate enpassant captures
            if (epsq != NoSq)
            {
                // lookup pawn attacks and bitwise AND with enpassant square (bit)
                Bitboard ep_captures = PawnAttacks[Black][fromSq] & (1ULL << epsq);
                
                // make sure enpassant capture available
                if (ep_captures)
                {
                    // init enpassant capture target square
                    int target_enpassant = ls1b(ep_captures);
                    addMove(MoveList, encodeMove(fromSq, target_enpassant, p, 0, 1, 0, 1, 0));
                }
            }
        }


        // White Knights
        else if (SqBB[fromSq] & bitboards[N])
        {
            // init piece attacks in order to get set of target squares
            captures = KnightAttacks[fromSq] & occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[Black] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, N, 0, 1, 0, 0, 0));
            }
        }


        // Black Knights
        else if (SqBB[fromSq] & bitboards[n])
        {
            // init piece attacks in order to get set of target squares
            captures = KnightAttacks[fromSq] & occupancies[White];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[White] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, n, 0, 1, 0, 0, 0));
            }
        }


        // White Bishops
        else if (SqBB[fromSq] & bitboards[B])
        {
            // init piece attacks in order to get set of target squares
            captures = getBishopAttacks(fromSq, occupancies[Both]) & occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[Black] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, B, 0, 1, 0, 0, 0));
            }
        }


        // Black Bishops
        else if (SqBB[fromSq] & bitboards[b])
        {
            // init piece attacks in order to get set of target squares
            captures = getBishopAttacks(fromSq, occupancies[Both]) & occupancies[White];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[White] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, b, 0, 1, 0, 0, 0));
            }
        }


        // White Rooks
        else if (SqBB[fromSq] & bitboards[R])
        {
            // init piece attacks in order to get set of target squares
            captures = getRookAttacks(fromSq, occupancies[Both]) & occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[Black] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, R, 0, 1, 0, 0, 0));
            }
        }


        // Black Rooks
        else if (SqBB[fromSq] & bitboards[r])
        {
            // init piece attacks in order to get set of target squares
            captures = getRookAttacks(fromSq, occupancies[Both]) & occupancies[White];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[White] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, r, 0, 1, 0, 0, 0));
            }
        }


        // White Queens
        else if (SqBB[fromSq] & bitboards[Q])
        {
            // init piece attacks in order to get set of target squares
            captures = getQueenAttacks(fromSq, occupancies[Both]) & occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[Black] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, Q, 0, 1, 0, 0, 0));
            }
        }


        // Black Queens
        else if (SqBB[fromSq] & bitboards[q])
        {
            // init piece attacks in order to get set of target squares
            captures = getQueenAttacks(fromSq, occupancies[Both]) & occupancies[White];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[White] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, q, 0, 1, 0, 0, 0));
            }
        }


        // White King
        else if (SqBB[fromSq] & bitboards[K])
        {
            // init piece attacks in order to get set of target squares
            captures = KingAttacks[fromSq] & occupancies[Black];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[Black] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, K, 0, 1, 0, 0, 0));
            }
        }


        // Black King
        else if (SqBB[fromSq] & bitboards[k])
        {
            // init piece attacks in order to get set of target squares
            captures = KingAttacks[fromSq] & occupancies[White];
            
            // loop over target squares available from generated attacks
            while (captures)
            {
                // init target square
                toSq = popLsb(captures);    
                
                // capture move
                if (occupancies[White] & SqBB[toSq])
                    addMove(MoveList, encodeMove(fromSq, toSq, k, 0, 1, 0, 0, 0));
            }
        }
    }
}



// printMoveList
//
// Print the list of generated pseudo-legal moves.
void printMoveList(MoveList_t &MoveList)
{
    // reliability check
    assert(MoveList.count < 0);


    // don't print anything when the move list is empty
    if (MoveList.count <= 0)
        return;
    

    // header text before listing the moves and their flags
    cout << endl;
    cout << "     move    piece    capture    double    ep    castle";
    cout << endl << endl;

    
    // loop over moves within a move list
    for (int i = 0; i < MoveList.count; i++)
    {
        // init move
        int move = MoveList.moves[i];

        
        // print move and piece
        cout << "     " << prettyMove(move) << "       ";
        if (!getPromo(move))
            cout << " ";


        // print piece being moved
        cout << PieceStr[getMovePiece(move)] << "          ";


        // print capture flag
        if (getMoveCapture(move)) cout << "1";
        else cout << "0";
        cout << "         ";


        // print double pawn push flag
        if (getDoublePush(move)) cout << "1";
        else cout << "0";
        cout << "     ";


        // print enpassant flag
        if (getEp(move)) cout << "1";
        else cout << "0";
        cout << "         ";


        // print castling flag
        if (getCastle(move)) cout << "1";
        else cout << "0";


        cout << endl;
    }


    // print total number of moves
    cout << endl << endl << "     Total number of moves: " << MoveList.count;
    cout << endl << endl;
}
