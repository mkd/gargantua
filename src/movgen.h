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

#ifndef MOVGEN_H
#define MOVGEN_H

#include <iostream>
#include <sstream>
#include <cassert>

#include "bitboard.h"
#include "position.h"
#include "tt.h"



// MoveList_t is a structure holding a list of moves (up to 256, which is
// enough for any legal chess position), and a pointer to the last element,
// which can also be used as a counter of elements in the move list.
typedef struct
{
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
    (toSq << 6) |       \
    (piece << 12) |     \
    (promo << 16) |     \
    (capture << 20) |   \
    (double << 21) |    \
    (ep << 22) |        \
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



// Table for bookkeeping castling rights when the King or Rook(s)
// move.
/*
                           castling   move     in      in
                              right update     binary  decimal

 king & rooks didn't move:     1111 & 1111  =  1111    15

        white king  moved:     1111 & 1100  =  1100    12
  white king's rook moved:     1111 & 1110  =  1110    14
 white queen's rook moved:     1111 & 1101  =  1101    13
     
         black king moved:     1111 & 0011  =  1011    3
  black king's rook moved:     1111 & 1011  =  1011    11
 black queen's rook moved:     1111 & 0111  =  0111    7

*/

// Castling rights update constants: when a rook or the King are moved,
// these rights are checked and the current castling rights are updated.
static constexpr int castling_rights[64] =
{
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};



// Functionality to generate and manipulate chess moves.
void generateMoves(MoveList_t &);
void generateCapturesAndPromotions(MoveList_t &);
void printMoveList(MoveList_t &);



// prettyMove
//
// Generate a string with the move in UCI notation.
static inline std::string prettyMove(int move)
{
    // string stream where to write the move to
    std::stringstream ss;


    // UCI format: [source][target][promotion] e.g., e2e4, e7e8q
    ss << SquareToCoordinates[getMoveSource(move)]
       << SquareToCoordinates[getMoveTarget(move)];

    if (getPromo(move))
        ss << PromoPieces[getPromo(move)];


    // return a string containing the move in UCI format
    return ss.str();
}



// printMove
//
// Print a move in UCI notation.
static inline void printMove(int move)
{
    std::cout << prettyMove(move);
}



// addMove
//
// Add a move to a move list.
static inline void addMove(MoveList_t &MoveList, int move)
{
    // reliability checks
    assert(MoveList.count >= 0);


    // strore move
    MoveList.moves[MoveList.count] = move;

    
    // increment move count
    MoveList.count++;
}



// isSquareAttacked
//
// True if the given square is attacked by any piece an opponent's piece.
constexpr bool isSquareAttacked(int square, int side)
{
    // reliability checks
    assert((side == White) || (side == Black));
    assert((square >= a8) && (square <= h1));


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



// saveBoard
//
// Implemented as a macro, it's job is to preserve the current board state
// in temporary variables.
#define saveBoard()                                                       \
    Bitboard bitboards_copy[12], occupancies_copy[3];                     \
    int side_copy, enpassant_copy, castle_copy, fifty_copy;               \
    memcpy(bitboards_copy, bitboards, sizeof(bitboards));                 \
    memcpy(occupancies_copy, occupancies, sizeof(occupancies));           \
    side_copy = sideToMove, enpassant_copy = epsq, castle_copy = castle;  \
    fifty_copy = fifty;                                                   \
    uint64_t hash_key_copy = hash_key;                                    \



// takeBack
//
// Implemented as a macro, it's job is to restore the previous board state
// from temporary variables. Because this is a macro, it needs to be used
// in conjunction with saveBoard() within the same scope.
#define takeBack()                                                        \
    memcpy(bitboards, bitboards_copy, sizeof(bitboards));                 \
    memcpy(occupancies, occupancies_copy, sizeof(occupancies));           \
    sideToMove = side_copy, epsq = enpassant_copy, castle = castle_copy;  \
    fifty = fifty_copy;                                                   \
    hash_key = hash_key_copy;                                             \



// makeMove
//
// Make move (thus alter the position) on the chess board.
//
// Note: remember to save the board status (saveBoard) before calling
//       makeMove(), if you then want to be able to use takeBack().
static inline int makeMove(int move)
{
    //reliability checks
    assert(move);
    assert((sideToMove == White) || (sideToMove == Black));


    // parse move components
    int fromSq   = getMoveSource(move);
    int toSq     = getMoveTarget(move);
    int piece    = getMovePiece(move);
    int promo    = getPromo(move);
    int capture  = getMoveCapture(move);
    int dpush    = getDoublePush(move);
    int ep       = getEp(move);
    int castling = getCastle(move);


    // configure opponent's color
    int Them     = White;
    if (sideToMove == White)
        Them = Black;
       

    // move the piece from source to target
    popBit(bitboards[piece], fromSq);
    setBit(bitboards[piece], toSq);


    // update occupancies for the piece being moved
    popBit(occupancies[sideToMove], fromSq);
    setBit(occupancies[sideToMove], toSq);


    // remove and set piece from source to target square in the hash key
    hash_key ^= piece_keys[piece][fromSq] ^ piece_keys[piece][toSq];


    // increment fifty move rule counter
    if ((piece != P) && (piece != p))
        fifty++;


    // handle castling moves
    if (castling)
    {
        // four different possibilities: white and black, 0-0 and 0-0-0
        switch (toSq)
        {
            // white castles king side (0-0)
            case (g1):
                // move rook from h1
                popBit(bitboards[R], h1);
                setBit(bitboards[R], f1);

                // update occupancies
                popBit(occupancies[White], h1);
                setBit(occupancies[White], f1);
                
                // hash rook
                hash_key ^= piece_keys[R][h1] ^ piece_keys[R][f1];

                break;
           

            // white castles queen side (0-0-0)
            case (c1):
                // move rook from a1
                popBit(bitboards[R], a1);
                setBit(bitboards[R], d1);

                // update occupancies
                popBit(occupancies[White], a1);
                setBit(occupancies[White], d1);
                
                // hash rook
                hash_key ^= piece_keys[R][a1] ^ piece_keys[R][d1];

                break;
           

            // black castles king side (0-0)
            case (g8):
                // move rook from h8
                popBit(bitboards[r], h8);
                setBit(bitboards[r], f8);

                // update occupancies
                popBit(occupancies[Black], h8);
                setBit(occupancies[Black], f8);
                
                // hash rook
                hash_key ^= piece_keys[r][h8] ^ piece_keys[r][f8];

                break;
           

            // black castles queen side (0-0-0)
            case (c8):
                // move rook from a8
                popBit(bitboards[r], a8);
                setBit(bitboards[r], d8);

                // update occupancies
                popBit(occupancies[Black], a8);
                setBit(occupancies[Black], d8);
                
                // hash rook
                hash_key ^= piece_keys[r][a8] ^ piece_keys[r][d8];

                break;
        }
    }
                

    // after the moving piece, also handle the captured piece, if any
    if (capture)
    {
        // reset fifty move rule counter
        fifty = 0;

        
        // pick up bitboard piece index ranges depending on side
        int start_piece, end_piece;
       

        // configure side to move (in order to reduce the piece)
        if (sideToMove == White)
        {
            start_piece = p;
            end_piece = k;
        }
        else
        {
            start_piece = P;
            end_piece = K;
        }

        
        // if there's a piece on the target, remove it from the bitboard
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
        {
            if (getBit(bitboards[bb_piece], toSq))
            {
                // remove the captured piece from the target square
                popBit(bitboards[bb_piece], toSq);

                // update occupancies for the piece just removed
                popBit(occupancies[Them], toSq);

                // remove the piece from hash key
                hash_key ^= piece_keys[bb_piece][toSq];

                break;
            }
        }


        // handle enpassant captures
        if (ep)
        {
            // white to move
            if (sideToMove == White)
            {
                // remove captured pawn
                popBit(bitboards[p], toSq + 8);

                // update occupancies
                popBit(occupancies[Black], toSq + 8);

                // remove pawn from hash key
                hash_key ^= piece_keys[p][toSq + 8];
            }
           

            // black to move
            else
            {
                // remove captured pawn
                popBit(bitboards[P], toSq - 8);

                // update occupancies
                popBit(occupancies[White], toSq - 8);

                // remove pawn from hash key
                hash_key ^= piece_keys[P][toSq - 8];
            }
        }
    }


    // handle pawn promotions
    if (promo)
    {
        // white to move
        if (sideToMove == White)
        {
            // erase the pawn from the target square
            popBit(bitboards[P], toSq);

            // remove pawn from hash key
            hash_key ^= piece_keys[P][toSq];
        }

        
        // black to move
        else
        {
            // erase the pawn from the target square
            popBit(bitboards[p], toSq);
            
            // remove pawn from hash key
            hash_key ^= piece_keys[p][toSq];
        }

        
        // set promoted piece on the chess board
        setBit(bitboards[promo], toSq);

        
        // add promoted piece into the hash key
        hash_key ^= piece_keys[promo][toSq];
    }



    // hash enpassant if available (remove enpassant square from hash key)
    if (epsq != NoSq)
        hash_key ^= enpassant_keys[epsq];
   

    // reset enpassant square
    epsq = NoSq;


    // handle double pawn pushes
    if (dpush)
    {
        // white to move
        if (sideToMove == White)
        {
            // set enpassant square
            epsq = toSq + 8;
            
            // hash enpassant
            hash_key ^= enpassant_keys[toSq + 8];
        }
        
        // black to move
        else
        {
            // set enpassant square
            epsq = toSq - 8;
            
            // hash enpassant
            hash_key ^= enpassant_keys[toSq - 8];
        }
    }


    // hash castling
    hash_key ^= castle_keys[castle];

    
    // update castling rights
    castle &= castling_rights[fromSq];
    castle &= castling_rights[toSq];


    // re-hash castling after updating castling rights
    hash_key ^= castle_keys[castle];


    // update all occupancies
    occupancies[Both] = occupancies[White] | occupancies[Black];


    // change side to move
    sideToMove ^= 1;
    hash_key ^= side_key;

    
    // check if move is legal (return 0 for illegal move, 1 for legal)
    if (!isSquareAttacked((sideToMove == White) ? ls1b(bitboards[k]) : ls1b(bitboards[K]), sideToMove))
        return 1;

    
    // return 0 as 'illegal move' if nothing happens
    return 0;
}



#endif  //  MOVGEN_H
