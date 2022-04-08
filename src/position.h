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

#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include <map>
#include <cstring>



// List of useful FEN positions used for testing and debbuging purposes
// @see https://www.chessprogramming.org/Perft_Results
#define FENPOS_EMPTYBOARD "8/8/8/8/8/8/8/8 b - - "
#define FENPOS_STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define FENPOS_KIWIPETE "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "
#define FENPOS_POS3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "
#define FENPOS_POS4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define FENPOS_POS5 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
#define FENPOS_POS6 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"



// ASCII piece strings lookup table. This table returns a readable
// string for each piece integer. E.g., 4=Q, 11=k, etc.
static const std::array<std::string, 12> PieceStr
{
    "P", "N", "B", "R", "Q", "K",
    "p", "n", "b", "r", "q", "k",
};



// Convert ASCII character pieces to encoded constants. This table
// converts char values to the piece integer. E.g., 'R' = R = 3.
static std::map<unsigned char, int> PieceConst =
{
    {'P', P},
    {'N', N},
    {'B', B},
    {'R', R},
    {'Q', Q},
    {'K', K},
    {'p', p},
    {'n', n},
    {'b', b},
    {'r', r},
    {'q', q},
    {'k', k}
};



// Table to lookup the color of a piece by using the piece itself as
// an index.
static std::map<int, Side> ColorFromPiece =
{
    {P, White},
    {N, White},
    {B, White},
    {R, White},
    {Q, White},
    {K, White},
    {p, Black},
    {n, Black},
    {b, Black},
    {r, Black},
    {q, Black},
    {k, Black}
};



// Array of promoted pieces, encoded from integer to character.
static std::map<int, unsigned char> PromoPieces = {
    {Q, 'q'},
    {R, 'r'},
    {B, 'b'},
    {N, 'n'},
    {q, 'q'},
    {r, 'r'},
    {b, 'b'},
    {n, 'n'}
};



// A chess position is defined by the following elements:
//
// 1. A set of 12 bitboards with all the piece occupancies
// 2. The side to move
// 3. The enpassant capture square
// 4. The castling rights
// 5. The 50-move rule counter
extern Bitboard bitboards[12];
extern Bitboard occupancies[3];
extern int sideToMove;
extern int epsq;
extern int castle;
extern int fifty;
extern int ply;



// Every chess position has its own (almost) unique hash key:
extern uint64_t hash_key;



// Flag to indicate whether the board should be displayed from White's
// perspective (false) or Black's perspective (true).
extern bool flip;



// Castling rights binary encoding
/*
    bin  dec
    
   0001    1  White King can castle to the king side
   0010    2  White King can castle to the queen side
   0100    4  Black King can castle to the king side
   1000    8  Black King can castle to the queen side


   Examples:

   1111       both sides an castle both directions
   1001       black king => queen side
              white king => king side
*/
enum castling { wk = 1, wq = 2, bk = 4, bq = 8 };



// Structures to detect 3-fold repetitions within the game:
//
// repetition_table stores a number of positions "played" during the search
// repetition_index tells the size of the repetition_table (pointer to last)
extern Bitboard repetition_table[1024];
extern int repetition_index;



// Functionality to handle a position on the chess board, including
// resting the board to its initial status, printing the board and
// parsing positions in FEN notation.
void resetBoard();
void printBoard();
void setPosition(const std::string &);
std::string getFEN();



// countRepetitions
//
// Counts the 3-fold repetitions played on the board. Returns the number
// of repetitions, i.e., a return value >= 3 means it's a draw.
static inline int countRepetitions()
{
    // reliability checks
    assert(ply > 0);


    // initialize variables to start counting positions
    int ilast, rep = 1;


    // we don't need to check the entire history, only positions
    // after the last pawn move or capture (i.e., ply - fifty)
    ilast = ply - fifty;


    // we can check every other position, because they are
    // alternating from side to side on the move
    for (int i = ply - 2; i >= ilast; i -= 2)
        if (repetition_table[i] == hash_key)
            rep++;
   

    // return the number of repetitions for this position
    return rep;
}



// isDraw
//
// Check if the current position is a draw given the following conditions:
// 1. Draw by 3-fold repetition
// 2. Draw by 50-move rule
// 3. Draw due to insufficient material
// 3.1 K-K ending
// 3.2 Kminor-K ending
// 3.3 KNN-K ending (not forced mate)
// 3.4 Kminor-Kminor ending 
// 3.5 KB-KB ending (all bishops on same-color squares)
// 3.6 KBN-Kminor
static inline bool isDraw()
{
    // 3-fold repetition
    if (countRepetitions() >= 3)
        return true;


    // 50-move rule
    if (fifty > 99)
        return true;


    // calculate the number of pieces on the board
    int total_pieces  = countBits(occupancies[Both]);
    int white_knights = countBits(bitboards[N]);
    int black_knights = countBits(bitboards[n]);
    int white_bishops = countBits(bitboards[B]);
    int black_bishops = countBits(bitboards[b]);

    
    // K-K ending
    if (total_pieces == 2)
        return true;


    // Kminor-K ending
    if (total_pieces == 3)
        if (white_knights || black_knights || white_bishops || black_bishops)
            return true;


    // KNN-K ending
    if (total_pieces == 4)
    {
        if (white_knights == 2)
            return true;

        else if (black_knights == 2)
            return true;

        // Kminor-Kminor ending
        else if ((white_knights || white_bishops) &&
                 (black_knights || black_bishops))
        {
            return true;
        }
    }


    // KB-KB ending (all bishops on same-color squares)
    if (((bitboards[B] | bitboards[b]) & LightSquares) ||
        ((bitboards[B] | bitboards[b]) & DarkSquares))
    {
        if ((bitboards[N] | bitboards[R] | bitboards[Q] | bitboards[P] |
             bitboards[n] | bitboards[r] | bitboards[q] | bitboards[p]) == 0)
        {
            return true;
        }
    }


    // Kminorminor-Kminor ending
    if ((total_pieces == 5) &&
        ((white_bishops + white_knights + black_bishops + black_knights) == 3))
    {
        // Strong bishop pair vs. Knight is not a draw
        if (white_bishops == 2)
        {
            if (((bitboards[B] & LightSquares) != bitboards[B]) &&
                (((bitboards[B] & DarkSquares) != bitboards[B])))
            return false;
        }

        if (black_bishops == 2)
        {
            if (((bitboards[b] & LightSquares) != bitboards[b]) &&
                (((bitboards[b] & DarkSquares) != bitboards[b])))
            return false;
        }


        // any other Kminor+minor vs. Kminor combination
        else
            return true;
    }


    // in any other case, it is not a draw
    return false;
}



// noMajorsOrMinors
//
// Return true if there are no major nor minor pieces left on the board.
static inline bool noMajorsOrMinors()
{
    return !(countBits(occupancies[Both]) - countBits(bitboards[P]) - countBits(bitboards[p]) - 2);
}



// get game phase score
/* bbc way
// 
    if (game_phase_score > opening_phase_score) game_phase = opening;
    else if (game_phase_score < endgame_phase_score) game_phase = endgame;
    else game_phase = middlegame;


static inline int get_game_phase_score()
{
        The game phase score of the game is derived from the pieces
        (not counting pawns and kings) that are still on the board.
        The full material starting position game phase score is:
        
        4 * knight material score in the opening +
        4 * bishop material score in the opening +
        4 * rook material score in the opening +
        2 * queen material score in the opening
    
    // white & black game phase scores
    int white_piece_scores = 0, black_piece_scores = 0;
    
    // loop over white pieces
    for (int piece = N; piece <= Q; piece++)
        white_piece_scores += count_bits(bitboards[piece]) * material_score[opening][piece];
    
    // loop over white pieces
    for (int piece = n; piece <= q; piece++)
        black_piece_scores += count_bits(bitboards[piece]) * -material_score[opening][piece];
    
    // return game phase score
    return white_piece_scores + black_piece_scores;
}
*/

/*
    chess0 way:

    // 2.2 Pieces
    whitepawns = bitCnt(board.whitePawns);
    whiteknights = bitCnt(board.whiteKnights);
    whitebishops = bitCnt(board.whiteBishops);
    whiterooks = bitCnt(board.whiteRooks);
    whitequeens = bitCnt(board.whiteQueens);
    whitetotalmat = 3 * whiteknights + 3 * whitebishops + 5 * whiterooks + 10 * whitequeens;
    whitetotal = whitepawns + whiteknights + whitebishops + whiterooks + whitequeens;
    blackpawns = bitCnt(board.blackPawns);
    blackknights = bitCnt(board.blackKnights);
    blackbishops = bitCnt(board.blackBishops);
    blackrooks = bitCnt(board.blackRooks);
    blackqueens = bitCnt(board.blackQueens);
    blacktotalmat = 3 * blackknights + 3 * blackbishops + 5 * blackrooks + 10 * blackqueens;
    blacktotal = blackpawns + blackknights + blackbishops + blackrooks + blackqueens;
    allpieces = board.whitePawns | board.whiteKnights | board.whiteBishops |
                board.whiteRooks | board.whiteQueens | board.whiteKing |
                board.blackPawns | board.blackKnights | board.blackBishops |
                board.blackRooks | board.blackQueens | board.blackKing;


    // 3. check if we are at the endgame: anything less than a
    //                                    queen + rook (=15), excluding pawns,
    //                                    is considered endgame
    endgame = (whitetotalmat < 15 || blacktotalmat < 15);
    */


#endif  //  POSITIION_H
