/*
  This file is part of Gargantua, a UCI chess engine with NNUE evaluation
  derived from Chess0, and inspired by Code Monkey King's bbc-1.4.

  Copyright (C) 2026 Claudio M. Camacho

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

#ifndef SEARCH_H
#define SEARCH_H

#include <atomic>
#include <cmath>
#include <cassert>
#include <chrono>
#include <future>
#include <string>
#include <thread>

#ifdef WIN64
#include <windows.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#endif

#include "movgen.h"
#include "thread.h"

using namespace std;

// Default settings and configuration for the search, as well as
// tuning parameters for search extensions and reductions:
constexpr int DefaultSearchDepth = 12;
constexpr int MaxSearchDepth = 256;
constexpr int DefaultMovetime = 5000;
constexpr int LMRFullDepthMoves = 4;
constexpr int LMRReductionLimit = 3;
constexpr int AspirationWindow = 70;
constexpr int WatchIntervalMs = 10;

constexpr uint64_t MaxSearchTime = 0xFFFFFFFFFFFFFFFFULL;

// Search definitions, including alpha-beta bounds, mating scores, etc.
constexpr int DrawScore = 0;
constexpr int MateValue = 49000;
constexpr int MateScore = 48000;
constexpr int ValueInfinite = 50000;

// Default options (settings) at startup
constexpr int OptionsDefaultHashSize = 1024;
constexpr int OptionsDefaultContempt = 25;
constexpr int OptionsContemptMin = 0;
constexpr int OptionsContemptMax = 200;

// Maximum depth at which we try to search


// Score assigned to non-capture promotions. This is used for
// sorting moves based on their likeliness to be good.
//
// @see scoreMove() and sortMoves()
constexpr int MoveScorePromoQuiet = 10000;



// Limits_t is a structure that holds the configuration of the search.
// This includes search depth, time to search, etc.
//
// The engine uses the global variable "limits" to set, edit and reset the
// search configuration throught the entire lifecycle.
typedef struct {
  int wtime;
  int btime;
  int winc;
  int binc;
  int npmsec;
  int movestogo;
  int depth;
  int mate;
  int perft;
  bool infinite;
  bool ponder;
  uint64_t movetime;
  uint64_t max_nodes;
} Limits_t;

extern Limits_t Limits;

// Map containing all the engine options that can be set using
// the UCI command 'setoption'
extern std::map<std::string, int> Options;

// Time Control variables
//
// These are flags to tell how the search is performed internally. These are not
// to be confused with Limits, which are UCI-specific settings parsed in the
// 'go' command.
extern uint64_t starttime;
extern uint64_t stoptime;
extern uint64_t inc;
extern std::atomic<bool> timedout;
extern bool timeset;



/*
      ================================
            Triangular PV table
      --------------------------------
        PV line: e2e4 e7e5 g1f3 b8c6
      ================================

           0    1    2    3    4    5

      0    m1   m2   m3   m4   m5   m6

      1    0    m2   m3   m4   m5   m6

      2    0    0    m3   m4   m5   m6

      3    0    0    0    m4   m5   m6

      4    0    0    0    0    m5   m6

      5    0    0    0    0    0    m6
*/



// Most Valuable Victim / Less Valuable Attacker (MVV/LVA) lookup table
/*

    (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600
*/

// MVV LVA [attacker][victim]
//
// A simple heuristic to generate or sort capture moves in a reasonable order.
// Inside a so called find-victim cycle, one first look up the potential
// victim of all attacked opponent pieces, in the order of the most valuable
// first, thus queen, rook, bishop, knight and pawn. After the most valuable
// victim is found, the find-aggressor cycle loops over the potential aggressors
// that may capture the victim in inverse order, from pawn, knight, bishop,
// rook, queen to king.
//
// @see https://www.chessprogramming.org/MVV-LVA
static constexpr int mvv_lva[12][12] = {
    {105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600},

    {105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600}};

// Functionality to search a position or perform an operation on the
// nodes of a given position.
void dperft(int);
void search();
int qsearch(int, int);
int see(int);
void initSearch();
void initLMR();
extern int LMRTable[MaxPly][256];
void sortMoves(MoveList_t &, int);
void printMoveScores(MoveList_t &);
void resetLimits();
void resetTimeControl();

// perft
//
// Verify move generation. All the leaf nodes up to the given depth are
// generated and counted.
//
// @see https://www.chessprogramming.org/Perft
static inline void perft(int depth) {
  // reliability checks
  assert(depth >= 0);

  // escape at leaf nodes and increment node count
  if (depth == 0) {
    nodes++;
    return;
  }

  // create move list instance
  MoveList_t MoveList;

  // generate moves
  generateMoves(MoveList);

  // loop over generated moves
  for (int move_count = 0; move_count < MoveList.count; move_count++) {
    // preserve board state
    saveBoard();

    // make move and, if illegal, skip to the next move
    NNUE_DO(MoveList.moves[move_count]);
    if (!makeMove(MoveList.moves[move_count])) {
      takeBack(MoveList.moves[move_count]);
      NNUE_UNDO(MoveList.moves[move_count]);
      continue;
    }

    // call perft driver recursively
    perft(depth - 1);

    // undo move
    takeBack(MoveList.moves[move_count]);
    NNUE_UNDO(MoveList.moves[move_count]);
  }
}

/*  =======================
         Move ordering
    =======================

    1. PV move
    2. Captures in MVV/LVA
    3. Promotions
    4. 1st killer move
    5. 2nd killer move
    6. History moves
    7. Unsorted moves
*/

// scoreMove
//
// Assign a score to a move.
static inline int scoreMove(int move) {
  // if PV move scoring is allowed
  // if PV move and scoring allowed, assign it the highest score
  if (scorePV && (pv_table[0][ply] == move)) {
    // disable score PV flag
    scorePV = false;

    // give PV move the highest score to search it first
    return 20000;
  }

  // score capture move
  else if (getMoveCapture(move)) {
    // init target piece
    int target_piece = P;
    int toSq = getMoveTarget(move);

    // pick up bitboard piece index ranges depending on side
    int start_piece = P, end_piece = K;

    if (sideToMove == White) {
      start_piece = p;
      end_piece = k;
    }

    // loop over the opponent's bitboards
    for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
      // if there's a piece on the target square
      if (getBit(bitboards[bb_piece], toSq)) {
        // remove it from corresponding bitboard
        target_piece = bb_piece;
        break;
      }
    }

    // score move by MVV LVA lookup [source piece][target piece]
    return mvv_lva[getMovePiece(move)][target_piece] + 10000;
  }

  // quiet promotions are also scored
  else if (getPromo(move)) {
    return MoveScorePromoQuiet;
  }

  // score quiet move
  else {
    // score 1st killer move
    if (killers[0][ply] == move)
      return 9000;

    // score 2nd killer move
    else if (killers[1][ply] == move)
      return 8000;

    // score countermove
    else if (ply > 0 && current_move[ply - 1] && countermoves[getMovePiece(current_move[ply - 1])][getMoveTarget(current_move[ply - 1])] == move)
      return 7500;

    // score history move
    else
      return history[getMovePiece(move)][getMoveTarget(move)];
  }

  // by default, don't add a score to the move
  return 0;
}

// enablePV_scoring
//
// Allow scoring PV moves.
static inline void enablePV_scoring(MoveList_t &MoveList) {
  // disable following PV
  followPV = false;

  // loop over the moves within a move list
  for (int count = 0; count < MoveList.count; count++) {
    // make sure we hit PV move
    if (pv_table[0][ply] == MoveList.moves[count]) {
      // enable move scoring and follow PV again
      scorePV = true;
      followPV = true;
    }
  }
}

// getTimeInMilliseconds
//
// Get the number of milliseconds since epoch time.
static inline uint64_t getTimeInMilliseconds() {
  return duration_cast<chrono::milliseconds>(
             chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

// inputWaiting
//
// Function to "listen" to GUI's input during the search, without
// blocking the program. Credit goes to Richard Albert, author of
// VICE chess engine.
static inline int inputWaiting() {
#ifndef WIN64

  fd_set readfds;
  struct timeval tv;
  FD_ZERO(&readfds);
  FD_SET(fileno(stdin), &readfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));

#else

  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init) {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    if (!pipe) {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }

  if (pipe) {
    if (!PeekNamedPipe(inh, nullptr, 0, nullptr, &dw, nullptr))
      return 1;
    return dw;
  }

  else {
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw <= 1 ? 0 : dw;
  }

#endif
}

// watchClockAndInput
//
// Check if we need to stop, because time is up, or because any other
// limit has been hit.
//
// This function should be called in its own (asynchronous) thread, e.g.,
// using launch::async with a future<> object.
static inline void watchClockAndInput() {
  string cmd;

  while (!timedout) {
    // watch clock
    if (timeset && (getTimeInMilliseconds() > stoptime))
      timedout = true;

    // read the input
    else if (inputWaiting()) {
      cin >> cmd;
      if (cmd == "stop")
        timedout = true;
    }

    // check for nodes    // Nodes
    else if ((Limits.max_nodes > 0) && (Threads.nodes_searched() > Limits.max_nodes))
      timedout = true;

    // update interval
    this_thread::sleep_for(chrono::milliseconds(WatchIntervalMs));
  }
}

// isEndgame
//
// Determine if the current position should be considered an endgame
// position for the current side to move.
static inline bool isEndgame() {
  int pawn_material = countBits(bitboards[P] | bitboards[p]) * 100;
  int knight_material = countBits(bitboards[N] | bitboards[n]) * 320;
  int bishop_material = countBits(bitboards[B] | bitboards[b]) * 320;
  int rook_material = countBits(bitboards[R] | bitboards[r]) * 500;
  int queen_material = countBits(bitboards[Q] | bitboards[q]) * 950;

  return ((pawn_material + knight_material + bishop_material + rook_material +
           queen_material) < 2600);
}

// contempt
//
// Determine the draw score based on the phase of the game and whose moving,
// to encourge the engine to strive for a win in the middle-game, but be
// satisified with a draw in the endgame.
static inline int contempt() {
  // in the endgame, it's ok to try to draw, if we're losing
  if (isEndgame())
    return DrawScore;

  // in the opening and middle game, we try to fight
  else
    return ((sideToMove == White) ? -Options["Contempt"] : Options["Contempt"]);
}

// mate_in
//
// Determine whether a mate has been found from root to leaf node.
constexpr int mate_in(int p) { return MateValue - p; }

// mated_in
//
// Determine whether a mate has been found from leaf node up to the root.
constexpr int mated_in(int p) { return -MateValue + p; }

// futility_margin
//
// Calculate a futility margin based on a given depth.
constexpr int futility_margin(int depth) { return 168 * depth; }

// futility_move_count
//
// Calculate how far in the sorted list of moves can we start pruning.
constexpr int futility_move_count(int depth) { return (3 + depth * depth) / 2; }

// getAttackers
//
// Create a Bitboard with all pieces from a given side attacking a given square.
static inline Bitboard getAttackers(Side color, int sq, Bitboard occupied) {
  // bitboards holding the attackers of different type
  Bitboard attackers = 0ULL;

  Bitboard attackingBishops = 0ULL;
  Bitboard attackingRooks = 0ULL;
  Bitboard attackingQueens = 0ULL;
  Bitboard attackingKnights = 0ULL;
  Bitboard attackingKings = 0ULL;
  Bitboard attackingPawns = 0ULL;

  // get the basic list of attackers
  attackingBishops = (color == White) ? bitboards[B] : bitboards[b];
  attackingRooks = (color == White) ? bitboards[R] : bitboards[r];
  attackingQueens = (color == White) ? bitboards[Q] : bitboards[q];
  attackingKnights = (color == White) ? bitboards[N] : bitboards[n];
  attackingKings = (color == White) ? bitboards[K] : bitboards[k];
  attackingPawns = (color == White) ? bitboards[P] : bitboards[p];

  // add long-range attacks
  Bitboard intercardinalRays = getBishopAttacks(sq, occupied);
  Bitboard cardinalRaysRays = getRookAttacks(sq, occupied);
  attackers |= intercardinalRays & (attackingBishops | attackingQueens);
  attackers |= cardinalRaysRays & (attackingRooks | attackingQueens);

  // add pawns and short-range pieces' attacks
  attackers |= KnightAttacks[sq] & attackingKnights;
  attackers |= KingAttacks[sq] & attackingKings;
  attackers |= PawnAttacks[color ^ 1][sq] & attackingPawns;

  // return a Bitboard containing all the attackers to the given square
  return attackers;
}

// considerXrays
//
// Find long-range attackers that attack a given square, in a given
// capturing sequence.
static inline Bitboard considerXrays(int sq, Bitboard occupied) {
  // start with an empty list (i.e., Bitboard) of attackers
  Bitboard attackers = 0ULL;

  // consider bishops, rooks and queens
  Bitboard attackingBishops = bitboards[B] | bitboards[b];
  Bitboard attackingRooks = bitboards[R] | bitboards[r];
  Bitboard attackingQueens = bitboards[Q] | bitboards[q];

  // add attacks from each long-range piece
  Bitboard intercardinalRays = getBishopAttacks(sq, occupied);
  Bitboard cardinalRaysRays = getRookAttacks(sq, occupied);

  attackers |= intercardinalRays & (attackingBishops | attackingQueens);
  attackers |= cardinalRaysRays & (attackingRooks | attackingQueens);

  // return the list of all possible X-ray (long-range) attackers
  return attackers;
}

// minAttacker
//
// Reveal the next (least valuable) attacker in a capturing sequence.
static inline Bitboard minAttacker(Bitboard attadef, int stm, int &attacker) {
  // decide what pieces to look for, depending on the side to move
  int start_piece = P, end_piece = K;

  if (stm == Black) {
    start_piece = p;
    end_piece = k;
  }

  // loop through the pieces to find the next attacker
  for (attacker = start_piece; attacker <= end_piece; attacker++) {
    Bitboard subset = attadef & bitboards[attacker];

    if (subset)
      return (subset & -subset);
  }

  // return an empty Bitboard, if no attackers left
  return 0;
}

#endif //  SEARCH_H
