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
    // Bitboard data and squares to generate moves
    int fromSq, toSq;
    Bitboard bb, attacks;


    // start with an empty move list
    MoveList.count = 0;

    
    // loop over all the bitboards
    for (int piece = P; piece <= k; piece++)
    {
        // init piece bitboard copy
        bb = bitboards[piece];


        // White Pawns and castling moves
        if (sideToMove == White)
        {
            // White Pawns
            if (piece == P)
            {
                while (bb)
                {
                    // init source square
                    fromSq = ls1b(bb);
                    
                    // init target square
                    toSq = fromSq - 8;
                    
                    // generate quiet pawn moves
                    if (!(toSq < a8) && !getBit(occupancies[Both], toSq))
                    {
                        // pawn promotions
                        if (SqBB[toSq] & Rank8_Mask)
                        {                            
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, Q, 0, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, R, 0, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, B, 0, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, N, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // one-square pawn push
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 0, 0, 0, 0));
                            
                            // double pawn push
                            if (((fromSq >= a2) && (fromSq <= h2)) && !getBit(occupancies[Both], (toSq - 8)))
                                addMove(MoveList, encodeMove(fromSq, (toSq - 8), piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = PawnAttacks[sideToMove][fromSq] & occupancies[Black];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        toSq = ls1b(attacks);
                        
                        // pawn promotions with capture
                        if (SqBB[toSq] & Rank8_Mask)
                        {
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, Q, 1, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, R, 1, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, B, 1, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, N, 1, 0, 0, 0));
                        }
                        
                        else
                            // one-square pawn push
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 1, 0, 0, 0));
                        
                        // pop ls1b from the pawn attacks
                        popBit(attacks, toSq);
                    }
                    
                    // generate enpassant captures
                    if (epsq != NoSq)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        Bitboard enpassant_attacks = PawnAttacks[sideToMove][fromSq] & (1ULL << epsq);
                        
                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = ls1b(enpassant_attacks);
                            addMove(MoveList, encodeMove(fromSq, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    popBit(bb, fromSq);
                }
            }

            
            // castling moves
            if (piece == K)
            {
                // White King short castle 0-0
                if (castle & wk)
                {
                    if (!(FG1_Mask & occupancies[Both]))
                    {
                        if (!isSquareAttacked(e1, Black) &&
                            !isSquareAttacked(f1, Black) &&
                            !isSquareAttacked(g1, Black))
                        {
                            addMove(MoveList, encodeMove(e1, g1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
                
                // White King long castle 0-0-0
                if (castle & wq)
                {
                    if (!(DCB1_Mask & occupancies[Both]))
                    {
                        if (!isSquareAttacked(e1, Black) &&
                            !isSquareAttacked(d1, Black) &&
                            !isSquareAttacked(c1, Black))
                        {
                            addMove(MoveList, encodeMove(e1, c1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }
        }
       

        // Black Pawns and castling moves
        else
        {
            // Black Pawns
            if (piece == p)
            {
                while (bb)
                {
                    // init source square
                    fromSq = ls1b(bb);
                    
                    // init target square
                    toSq = fromSq + 8;
                    
                    // generate quiet pawn moves
                    if (!(toSq > h1) && !getBit(occupancies[Both], toSq))
                    {
                        // pawn promotions
                        if (SqBB[toSq] & Rank1_Mask)
                        {
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, q, 0, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, r, 0, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, b, 0, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, n, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // one-square pawn push
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 0, 0, 0, 0));
                            
                            // double pawn push
                            if (((fromSq >= a7) && (fromSq <= h7)) && !getBit(occupancies[Both], (toSq + 8)))
                                addMove(MoveList, encodeMove(fromSq, (toSq + 8), piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = PawnAttacks[sideToMove][fromSq] & occupancies[White];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        toSq = ls1b(attacks);
                        
                        // pawn promotions with capture
                        if (SqBB[toSq] & Rank1_Mask)
                        {
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, q, 1, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, r, 1, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, b, 1, 0, 0, 0));
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, n, 1, 0, 0, 0));
                        }
                        
                        else
                            // one-square ahead pawn move
                            addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 1, 0, 0, 0));
                        
                        // pop ls1b from the pawn attacks
                        popBit(attacks, toSq);
                    }

                    // generate enpassant captures
                    if (epsq != NoSq)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        Bitboard enpassant_attacks = PawnAttacks[sideToMove][fromSq] & (1ULL << epsq);
                        
                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = ls1b(enpassant_attacks);
                            addMove(MoveList, encodeMove(fromSq, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    popBit(bb, fromSq);
                }
            }
           

            // castling moves
            if (piece == k)
            {
                // Black King short castle 0-0
                if (castle & bk)
                {
                    if (!(FG8_Mask & occupancies[Both]))
                    {
                        if (!isSquareAttacked(e8, White) &&
                            !isSquareAttacked(f8, White) &&
                            !isSquareAttacked(g8, White))
                        {
                            addMove(MoveList, encodeMove(e8, g8, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
                
                // Black King long castle 0-0-0
                if (castle & bq)
                {
                    if (!(DCB8_Mask & occupancies[Both]))
                    {
                        if (!isSquareAttacked(e8, White) &&
                            !isSquareAttacked(d8, White) &&
                            !isSquareAttacked(c8, White))
                        {
                            addMove(MoveList, encodeMove(e8, c8, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }
        }
       

        // Knight moves
        if ((sideToMove == White) ? piece == N : piece == n)
        {
            while (bb)
            {
                // init source square
                fromSq = ls1b(bb);
                
                // init piece attacks in order to get set of target squares
                attacks = KnightAttacks[fromSq] & ~occupancies[sideToMove];
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    toSq = ls1b(attacks);    
                    
                    // quiet move
                    if (!getBit(((sideToMove == White) ? occupancies[Black] : occupancies[White]), toSq))
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b from the current attacks set
                    popBit(attacks, toSq);
                }
                
                // pop ls1b from the current piece bitboard copy
                popBit(bb, fromSq);
            }
        }
       

        // Bishop moves
        if ((sideToMove == White) ? piece == B : piece == b)
        {
            while (bb)
            {
                // init source square
                fromSq = ls1b(bb);
                
                // init piece attacks in order to get set of target squares
                attacks = getBishopAttacks(fromSq, occupancies[Both]) & ~occupancies[sideToMove];
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    toSq = ls1b(attacks);    
                    
                    // quiet move
                    if (!getBit(((sideToMove == White) ? occupancies[Black] : occupancies[White]), toSq))
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b from the current attacks set
                    popBit(attacks, toSq);
                }
                
                // pop ls1b of the current piece bitboard copy
                popBit(bb, fromSq);
            }
        }
       

        // Rook moves
        if ((sideToMove == White) ? piece == R : piece == r)
        {
            while (bb)
            {
                // init source square
                fromSq = ls1b(bb);
                
                // init piece attacks in order to get set of target squares
                attacks = getRookAttacks(fromSq, occupancies[Both]) & ~occupancies[sideToMove];
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    toSq = ls1b(attacks);    
                    
                    // quiet move
                    if (!getBit(((sideToMove == White) ? occupancies[Black] : occupancies[White]), toSq))
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b in current attacks set
                    popBit(attacks, toSq);
                }
                
                // pop ls1b from the current piece bitboard copy
                popBit(bb, fromSq);
            }
        }

        
        // Queen moves
        if ((sideToMove == White) ? piece == Q : piece == q)
        {
            while (bb)
            {
                // init source square
                fromSq = ls1b(bb);
                
                // init piece attacks in order to get set of target squares
                attacks = getQueenAttacks(fromSq, occupancies[Both]) & ~occupancies[sideToMove];
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    toSq = ls1b(attacks);    
                    
                    // quiet move
                    if (!getBit(((sideToMove == White) ? occupancies[Black] : occupancies[White]), toSq))
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b from the current attacks set
                    popBit(attacks, toSq);
                }
                
                // pop ls1b from the current piece bitboard copy
                popBit(bb, fromSq);
            }
        }


        // King moves
        if ((sideToMove == White) ? piece == K : piece == k)
        {
            while (bb)
            {
                // init source square
                fromSq = ls1b(bb);
                
                // init piece attacks in order to get set of target squares
                attacks = KingAttacks[fromSq] & ~occupancies[sideToMove];
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    toSq = ls1b(attacks);    
                    
                    // quiet move
                    if (!getBit(((sideToMove == White) ? occupancies[Black] : occupancies[White]), toSq))
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        addMove(MoveList, encodeMove(fromSq, toSq, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b from current attacks set
                    popBit(attacks, toSq);
                }

                // pop ls1b from the current piece bitboard copy
                popBit(bb, fromSq);
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
        cout << "     " << SquareToCoordinates[getMoveSource(move)]
                        << SquareToCoordinates[getMoveTarget(move)]
                        << PromoPieces[getPromo(move)] << "   "
                        << PieceStr[getMovePiece(move)] << "        ";


        // print capture flag
        if (getMoveCapture(move)) cout << "1";
        else cout << "0";
        cout << "          ";


        // print double pawn push flag
        if (getDoublePush(move)) cout << "1";
        else cout << "0";
        cout << "         ";


        // print enpassant flag
        if (getEp(move)) cout << "1";
        else cout << "0";
        cout << "     ";


        // print castling flag
        if (getCastle(move)) cout << "1";
        else cout << "0";


        cout << endl;
    }


    // print total number of moves
    cout << endl << endl << "     Total number of moves: " << MoveList.count;
    cout << endl << endl;
}



// gen2
//
// Alternative implementation to generateMoves().
void gen2(MoveList_t &MoveList)
{
    Bitboard Us = occupancies[sideToMove];
    int fromSq = NoSq, toSq = NoSq;
    Bitboard attacks = 0ULL;


    // start with an empty move list
    MoveList.count = 0;


    while (Us)
    {
        fromSq = popLsb(Us);

        // white pawns
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

        // black pawns
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

        // white knights
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

        // black knights
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

        // white bishops
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

        // black bishops
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

        // white rooks
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

        // black rooks
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

        // white queens
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

        // black queens
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

        // white king
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

        // black king
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
