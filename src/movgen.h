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

#ifndef MOVGEN_H
#define MOVGEN_H

#include <vector>

#include "bitboard.h"
#include "position.h"



// Move list structure where to store the list of generated moves
extern std::vector<int> MoveList;

// alternative using C 
// TODO: check array vs. vector implementation. Which is faster?
typedef struct {
    int moves[256];
    int count;
} MoveList_t;




// Move encoding:
//
// Moves are encoded using 24 bits, where the following schema is followed:
/*
          binary move bits                               hexadecimal constants
    
    0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0001 0000 0000 0000 0000 0000    capture flag        0x100000
    0010 0000 0000 0000 0000 0000    double push flag    0x200000
    0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
    1000 0000 0000 0000 0000 0000    castling flag       0x800000
*/

// Encode move macro
#define encodeMove(fromSq, toSq, piece, promo, capture, double, ep, castling) \
    (fromSq) |          \
    (toSq << 6) |     \
    (piece << 12) |     \
    (promo << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (ep << 22) | \
    (castling << 23)    \


// Extract source square
#define getMoveSource(move) (move & 0x3f)


// Extract target square
#define getMoveTarget(move) ((move & 0xfc0) >> 6)


// Extract piece
#define getMovePiece(move) ((move & 0xf000) >> 12)


// Extract promoted piece
#define getPromo(move) ((move & 0xf0000) >> 16)


// Extract capture flag
#define getMoveCapture(move) (move & 0x100000)


// Extract double pawn push flag
#define getDoublePush(move) (move & 0x200000)


// Extract enpassant flag
#define getEp(move) (move & 0x400000)


// Extract castling flag
#define getCastle(move) (move & 0x800000)



// Functionality to generate and manipulate chess moves.
void generateMoves();
void printMoveList(std::vector<int> &);



// print_move
static inline void print_move(int move)
{
    std::cout << SquareToCoordinates[getMoveSource(move)]
              << SquareToCoordinates[getMoveTarget(move)]
              << PromoPieces[getPromo(move)];
}


// add move to the move list
static inline void add_move(MoveList_t *move_list, int move)
{
    // strore move
    move_list->moves[move_list->count] = move;
    
    // increment move count
    move_list->count++;
}


// print move list
void print_move_list(MoveList_t *);






// isSquareAttacked
//
// True if the given square is attacked by any piece an opponent's piece.
static inline bool isSquareAttacked(int square, int side)
{
    // square attacked by White or Black pawns
    if ((side == White) && (PawnAttacks[Black][square] & bitboards[P]))
        return true;
    
    if ((side == Black) && (PawnAttacks[White][square] & bitboards[p]))
        return true;
   

    // square attacked by Knights
    if (KnightAttacks[square] & ((side == White) ? bitboards[N] : bitboards[n]))
        return true;

    
    // square attacked by Bishops
    if (getBishopAttacks(square, occupancies[Both]) & ((side == White) ? bitboards[B] : bitboards[b]))
        return true;


    // square attacked by Rooks
    if (getRookAttacks(square, occupancies[Both]) & ((side == White) ? bitboards[R] : bitboards[r]))
        return true;


    // square attacked by Queens
    if (getQueenAttacks(square, occupancies[Both]) & ((side == White) ? bitboards[Q] : bitboards[q]))
        return true;

    
    // square attacked by Kings
    if (KingAttacks[square] & ((side == White) ? bitboards[K] : bitboards[k]))
        return true;


    // by default return false
    return false;
}



#endif  //  MOVGEN_H
