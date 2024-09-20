/*
  This file is part of Gargantua, a UCI chess engine with NNUE evaluation
  derived from Chess0, and inspired by Code Monkey King's bbc-1.4.
     
  Copyright (C) 2024 Claudio M. Camacho
 
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
#include <cstring>
#include <cassert>

#include "bitboard.h"
#include "tt.h"



using namespace std;



// A map to transform each square number into a Bitboard with only 1 bit set.
// The NoSq square is at index [64] and is an empty Bitboard.
Bitboard SqBB[65];



// Leapers' attack (and move) tables [color][square]
Bitboard PawnPushes[2][64];
Bitboard PawnDoublePushes[2][64];
Bitboard PawnAttacks[2][64];
Bitboard KnightAttacks[64];
Bitboard KingAttacks[64];



// Slider masks and attack tables
Bitboard BishopMasks[64];
Bitboard RookMasks[64];
Bitboard BishopAttacks[64][512];
Bitboard RookAttacks[64][4096];



// Pseudo-random number generator seed
uint32_t rng32_state = 1804289383;



// genPawnPush
//
// Generate a Bitboard with the push of a pawn at a given square.
Bitboard genPawnPush(int side, int square)
{
    // reliability checks
    assert((side == White) || (side == Black));
    assert((square >= a8) && (square <= h1));


    // result pushes Bitboard
    Bitboard push = 0ULL;


    // piece Bitboard
    Bitboard bb = 0ULL;


    // set piece on board
    setBit(bb, square);


    // white pawns
    if (side == White)
        push |= (bb >> 8);


    // black pawns
    else if (side == Black)
        push |= (bb << 8);


    // return push map
    return push;
}



// genPawnDoublePush
//
// Generate a Bitboard with the double push of a pawn at a given square.
Bitboard genPawnDoublePush(int side, int square)
{
    // reliability checks
    assert((side == White) || (side == Black));
    assert((square >= a8) && (square <= h1));


    // result pushes Bitboard
    Bitboard push = 0ULL;


    // piece Bitboard
    Bitboard bb = 0ULL;


    // set piece on board
    setBit(bb, square);


    // white pawns
    if (side == White)
    {
        if (bb & Rank2_Mask)
            push |= (bb >> 16);
    }


    // black pawns
    else if (side == Black)
    {
        if (bb & Rank7_Mask)
            push |= (bb << 16);
    }


    // return push map
    return push;
}



// maskPawnAttacks
//
// Generate a Bitboard with all Pawn attacks from a given square.
Bitboard maskPawnAttacks(int side, int square)
{
    // reliability checks
    assert((side == White) || (side == Black));
    assert((square >= a8) && (square <= h1));


    // result attacks Bitboard
    Bitboard attacks = 0ULL;


    // piece Bitboard
    Bitboard bb = 0ULL;


    // set piece on board
    setBit(bb, square);


    // white pawns
    if (side == White)
    {
        if ((bb >> 7) & NotFileA_Mask) attacks |= (bb >> 7);
        if ((bb >> 9) & NotFileH_Mask) attacks |= (bb >> 9);
    }


    // black pawns
    else if (side == Black)
    {
        if ((bb << 7) & NotFileH_Mask) attacks |= (bb << 7);
        if ((bb << 9) & NotFileA_Mask) attacks |= (bb << 9);    
    }


    // return attack map
    return attacks;
}



// maskKnightAttacks
//
// Generate a Bitboard with all Knight attacks from a given square.
Bitboard maskKnightAttacks(int square)
{
    // reliability checks
    assert((square >= a8) && (square <= h1));


    // result attacks bitboard
    Bitboard attacks = 0ULL;


    // piece bitboard
    Bitboard bb = 0ULL;


    // set piece on board
    setBit(bb, square);


    // generate Knight attacks
    if ((bb >> 17) & NotFileH_Mask)  attacks |= (bb >> 17);
    if ((bb >> 15) & NotFileA_Mask)  attacks |= (bb >> 15);
    if ((bb >> 10) & NotFileHG_Mask) attacks |= (bb >> 10);
    if ((bb >> 6)  & NotFileAB_Mask) attacks |= (bb >> 6);
    if ((bb << 17) & NotFileA_Mask)  attacks |= (bb << 17);
    if ((bb << 15) & NotFileH_Mask)  attacks |= (bb << 15);
    if ((bb << 10) & NotFileAB_Mask) attacks |= (bb << 10);
    if ((bb << 6)  & NotFileHG_Mask) attacks |= (bb << 6);


    // return attack map
    return attacks;
}



// maskKingAttacks
//
// Generate a Bitboard with all King attacks from a given square.
Bitboard maskKingAttacks(int square)
{
    // reliability checks
    assert((square >= a8) && (square <= h1));


    // result attacks bitboard
    Bitboard attacks = 0ULL;


    // piece bitboard
    Bitboard bb = 0ULL;


    // set piece on board
    setBit(bb, square);


    // generate King attacks
    if (bb >> 8)                   attacks |= (bb >> 8);
    if ((bb >> 9) & NotFileH_Mask) attacks |= (bb >> 9);
    if ((bb >> 7) & NotFileA_Mask) attacks |= (bb >> 7);
    if ((bb >> 1) & NotFileH_Mask) attacks |= (bb >> 1);
    if (bb << 8)                   attacks |= (bb << 8);
    if ((bb << 9) & NotFileA_Mask) attacks |= (bb << 9);
    if ((bb << 7) & NotFileH_Mask) attacks |= (bb << 7);
    if ((bb << 1) & NotFileA_Mask) attacks |= (bb << 1);


    // return attack map
    return attacks;
}



// maskBishopAttacks
//
// Generate a Bitboard with the occupancy bits of a bishop at a given square.
Bitboard maskBishopAttacks(int square)
{
    // reliability checks
    assert((square >= a8) && (square <= h1));


    // result attacks bitboard
    Bitboard attacks = 0ULL;


    // rank and file, plus target ranks and files (of the diagonals)
    int r, f;
    int tr = square / 8;
    int tf = square % 8;


    // mask relevant Bishop occupancy bits
    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
        attacks |= (1ULL << (r * 8 + f));

    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
        attacks |= (1ULL << (r * 8 + f));

    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
        attacks |= (1ULL << (r * 8 + f));

    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
        attacks |= (1ULL << (r * 8 + f));


    // return attack map
    return attacks;
}



// maskRookAttacks
//
// Generate a Bitboard with the occupancy bits of a rook at a given square.
Bitboard maskRookAttacks(int square)
{
    // reliability checks
    assert((square >= a8) && (square <= h1));


    // result attacks bitboard
    Bitboard attacks = 0ULL;


    // rank and file, plus target ranks and files (of the diagonals)
    int r, f;
    int tr = square / 8;
    int tf = square % 8;


    // mask relevant Rook occupancy bits
    for (r = tr + 1; r <= 6; r++)
        attacks |= (1ULL << (r * 8 + tf));

    for (r = tr - 1; r >= 1; r--)
        attacks |= (1ULL << (r * 8 + tf));

    for (f = tf + 1; f <= 6; f++)
        attacks |= (1ULL << (tr * 8 + f));

    for (f = tf - 1; f >= 1; f--)
        attacks |= (1ULL << (tr * 8 + f));


    // return attack map
    return attacks;
}



// genBishopAttacks
//
// Generate a Bitboard with the Bishop attacks given a position and a set of
// blocking pieces.
Bitboard genBishopAttacks(int square, Bitboard block)
{
    // reliability checks
    assert((square >= a8) && (square <= h1));


    // result attacks bitboard
    Bitboard attacks = 0ULL;


    // rank and file, plus target ranks and files (of the diagonals)
    int r, f;
    int tr = square / 8;
    int tf = square % 8;


    // generate Bishop atacks
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block)
            break;
    }
    
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block)
            break;
    }
    
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block)
            break;
    }
    
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block)
            break;
    }


    // return attack map
    return attacks;
}



// genRookAttacks
//
// Generate a Bitboard with the Rook attacks given a position and a set of
// blocking pieces.
Bitboard genRookAttacks(int square, Bitboard block)
{
    // reliability checks
    assert((square >= a8) && (square <= h1));


    // result attacks bitboard
    Bitboard attacks = 0ULL;
  

    // rank and file, plus target ranks and files (of the diagonals)
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
  

    // generate rook attacks
    for (r = tr + 1; r <= 7; r++)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block)
            break;
    }
    
    for (r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block)
            break;
    }
    
    for (f = tf + 1; f <= 7; f++)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block)
            break;
    }
    
    for (f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block)
            break;
    }
  

    // return attack map
    return attacks;
}



// setOccupancy
//
// Set the occupancy bits based on the pieces on the board.
Bitboard setOccupancy(int index, int bitMask, Bitboard attackMask)
{
    // occupancy map
    Bitboard occupancy = 0ULL;

    
    // loop over the range of bits within attack mask
    for (int count = 0; count < bitMask; count++)
    {
        // get LS1B index of attacks mask
        int square = ls1b(attackMask);

        // pop LS1B in attack map
        attackMask ^= SqBB[square];
        
        // make sure occupancy is on board and populate occupancy map
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }
   

    // return occupancy map
    return occupancy;
}



// initBitmaps
//
// Initialize different bitmap tables for faster lookup.
void initBitmaps()
{
    // populate SqBB
    memset(SqBB, 0ULL, sizeof(SqBB));
    for (int square = 0; square < 64; square++)
        setBit(SqBB[square], square);
}



// initLeaperAttacks
//
// Initialize the leaper pieces' attacks.
void initLeaperAttacks()
{
    // loop over the 64 board squares
    for (int square = 0; square < 64; square++)
    {
        // init Pawn pushes and attacks
        PawnPushes[White][square] = genPawnPush(White, square);
        PawnPushes[Black][square] = genPawnPush(Black, square);
        PawnDoublePushes[White][square] = genPawnDoublePush(White, square);
        PawnDoublePushes[Black][square] = genPawnDoublePush(Black, square);
        PawnAttacks[White][square] = maskPawnAttacks(White, square);
        PawnAttacks[Black][square] = maskPawnAttacks(Black, square);


        // init Knight attacks
        KnightAttacks[square] = maskKnightAttacks(square);


        // init King attacks
        KingAttacks[square] = maskKingAttacks(square);
    }
}



// initSliderAttacks
//
// Initialize the slider pieces' attacks.
void initSliderAttacks(Slider isBishop)
{
    // loop over the 64 board squares
    for (int square = 0; square < 64; square++)
    {
        // init Bishop & Rook masks
        BishopMasks[square] = maskBishopAttacks(square);
        RookMasks[square] = maskRookAttacks(square);

        
        // init current mask
        Bitboard AttackMask = isBishop ? BishopMasks[square] : RookMasks[square];

        
        // init relevant occupancy bit count
        int RelevantBitsCount = countBits(AttackMask);

        
        // init occupancy indices
        int OccupancyIndices = (1 << RelevantBitsCount);

        
        // loop over occupancy indices
        for (int index = 0; index < OccupancyIndices; index++)
        {
            // Bishop
            if (isBishop)
            {
                // init current occupancy variation
                Bitboard occupancy = setOccupancy(index, RelevantBitsCount, AttackMask);
                
                // init magic index
                int MagicIndex = (occupancy * BishopMagicNumbers[square]) >> (64 - BishopRelevantBits[square]);
                
                // init bishop attacks
                BishopAttacks[square][MagicIndex] = genBishopAttacks(square, occupancy);
            }

            
            // Rook
            else
            {
                // init current occupancy variation
                Bitboard occupancy = setOccupancy(index, RelevantBitsCount, AttackMask);
                
                // init magic index
                int MagicIndex = (occupancy * RookMagicNumbers[square]) >> (64 - RookRelevantBits[square]);
                
                // init rook attacks
                RookAttacks[square][MagicIndex] = genRookAttacks(square, occupancy);
            }
        }
    }
}



// printBitboard
//
// Take a Bitboard and display it as a board representation on the standard
// output.
void printBitboard(Bitboard bb)
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

            // print bit state (either 1 or 0)
            cout << " " << (getBit(bb, square) ? " 1 " : " . ") << "|";
        }


        // print new line every rank
        cout << endl << "    +----+----+----+----+----+----+----+----+" << endl;
    }


    // print board files
    cout << "      a    b    c    d    e    f    g    h" << endl << endl;


    // print Bitboard details
    cout << endl << "Bitboard: " << bb << "ULL" << endl << endl;
}



// initBitboards
//
// Call all different functions that initialize fundamental data structures
// and Bitboards to make the engine work.
void initBitboards()
{
    initBitmaps();
    initLeaperAttacks();
    initSliderAttacks(Bishop);
    initSliderAttacks(Rook);
}
