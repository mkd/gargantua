/*
  This file is part of Gargantua...
*/

#include "eval.h"
#include "position.h"
#include "stockfish_probe/nnue_incremental.h"
#include "stockfish_probe/probe.h"

// Mapping from Gargantua pieces (P=0...k=11) to Stockfish Probe pieces
// (1..6, 9..14)
static int g_to_sf[12] = {
    1, 2,  3,  4,  5,  6, // P N B R Q K -> 1 2 3 4 5 6
    9, 10, 11, 12, 13, 14 // p n b r q k -> 9 10 11 12 13 14
};

int evaluate() {
  Bitboard bb;
  int piece, square;
  int pieces[33];
  int squares[33];
  int index = 2;

  for (int bb_piece = P; bb_piece <= k; bb_piece++) {
    bb = bitboards[bb_piece];
    while (bb) {
      piece = bb_piece;
      square = popLsb(bb);

      int sf_piece = g_to_sf[piece];
      // Gargantua squares: a8=0 ... h1=63 (Rank-major, top-down)
      // Stockfish squares: a1=0 ... h8=63 (Rank-major, bottom-up)
      // Fix: XOR with 56 (111000) to flip the rank bits.
      int sf_square = square ^ 56;

      if (piece == K) {
        pieces[0] = sf_piece;
        squares[0] = sf_square;
      } else if (piece == k) {
        pieces[1] = sf_piece;
        squares[1] = sf_square;
      } else {
        pieces[index] = sf_piece;
        squares[index] = sf_square;
        index++;
      }
    }
  }

  pieces[index] = 0;
  squares[index] = 0;

  return Stockfish::Incremental::evaluate() * (100 - fifty) / 100;
}
