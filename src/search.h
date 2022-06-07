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

#ifndef SEARCH_H
#define SEARCH_H

#include <cassert>
#include <chrono>
#include <string>
#include <thread>
#include <future>

#ifdef WIN64
    #include <windows.h>
#else
    #include <sys/time.h>
    #include <sys/select.h>
#endif

#include "movgen.h"



using namespace std;



// Default settings and configuration for the search, as well as 
// tuning parameters for search extensions and reductions:
#define DefaultSearchDepth           12
#define MaxSearchDepth              256
#define DefaultMovetime            5000
#define LMRFullDepthMoves             4
#define LMRReductionLimit             3
#define AspirationWindow            200
#define WatchIntervalMs              10

#define MaxSearchTime  0xFFFFFFFFFFFFFFFFULL



// Search definitions, including alpha-beta bounds, mating scores, etc.
#define DrawScore           0
#define MateValue       49000
#define MateScore       48000
#define ValueInfinite   50000




// Default options (settings) at startup
#define OptionsDefaultHashSize      1024 
#define OptionsDefaultContempt        25
#define OptionsContemptMin             0
#define OptionsContemptMax           200



// Maximum depth at which we try to search
#define MaxPly            256



// Score assigned to non-capture promotions. This is used for
// sorting moves based on their likeliness to be good.
//
// @see scoreMove() and sortMoves()
#define MoveScorePromoQuiet   10000



// 'nodes' is a global variable holding the number of nodes analyzed
// or searched. It is used by negamax() but also other performance test
// functions such as perft().
extern uint64_t nodes;



// Limits_t is a structure that holds the configuration of the search.
// This includes search depth, time to search, etc.
//
// The engine uses the global variable "limits" to set, edit and reset the
// search configuration throught the entire lifecycle.
typedef struct
{
    int  wtime;
    int  btime;
    int  winc;
    int  binc;
    int  npmsec;
    int  movestogo;
    int  depth;
    int  mate;
    int  perft;
    bool infinite;
    bool ponder;
    uint64_t movetime;
    uint64_t nodes;
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
extern bool     timedout;
extern bool     timeset;



// killers [id][ply] 
//
// Killers is a table where the two best (quiet) moves are
// systematically stored for later searches. This is based on the
// fact that a move producing a beta cut-off must be a good one.
// beta cut-offs, where a move killer moves [id][ply]
//
// Note: storing exactly 2 killer moves is best for efficiency/performance.
//
// @see https://www.chessprogramming.org/Killer_Heuristic
extern int killers[2][MaxPly];



// history [piece][square]
//
// History is a table where to store moves that have produced an improvement in
// the score of previous searches. In other words, they have raised alpha.
//
// @see https://www.chessprogramming.org/History_Heuristic
extern int history[12][64];



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

// PV length [ply]
//
// An array of Principal Variations indexed by ply (distance to root). This is
// employed to collect the principal variation of best moves inside the
// alpha-beta or principal variation search, with the best score moves
// propagated up to the root.
//
// @see https://www.chessprogramming.org/Triangular_PV-Table
extern int pv_length[MaxPly];



// PV table [ply][ply]
extern int pv_table[MaxPly][MaxPly];



// follow PV & score PV move
extern bool followPV, scorePV;



// flag to control whether we allow null move pruning or not
extern bool allowNull;



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
// that may capture the victim in inverse order, from pawn, knight, bishop, rook,
// queen to king. 
//
// @see https://www.chessprogramming.org/MVV-LVA
static constexpr int mvv_lva[12][12] =
{
 	{105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600},

	{105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600}
};



// Functionality to search a position or perform an operation on the
// nodes of a given position.
void dperft(int);
void search();
int  qsearch(int, int);
void initSearch();
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
static inline void perft(int depth)
{
    // reliability checks
    assert(depth >= 0);


    // escape at leaf nodes and increment node count
    if (depth == 0)
    {
        nodes++;
        return;
    }

    
    // create move list instance
    MoveList_t MoveList;

    
    // generate moves
    generateMoves(MoveList);

    
    // loop over generated moves
    for (int move_count = 0; move_count < MoveList.count; move_count++)
    {   
        // preserve board state
        saveBoard();


        // make move and, if illegal, skip to the next move
        if (!makeMove(MoveList.moves[move_count]))
        {
            takeBack();
            continue;
        }


        // call perft driver recursively
        perft(depth - 1);

        
        // undo move
        takeBack();
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
static inline int scoreMove(int move)
{
    // if PV move scoring is allowed
    // if PV move and scoring allowed, assign it the highest score
    if (scorePV && (pv_table[0][ply] == move))
    {
        // disable score PV flag
        scorePV = false;
         
        // give PV move the highest score to search it first
        return 20000;
    }


    // score capture move
    //
    // TODO: replace with static exchange evaluation -- see()
    //       --> sort moves based on see() value + 10000
    else if (getMoveCapture(move))
    {
        // init target piece
        int target_piece = P;
        int toSq = getMoveTarget(move);

        
        // pick up bitboard piece index ranges depending on side
        int start_piece, end_piece;

        
        // pick up side to move
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

        
        // loop over the opponent's bitboards
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
        {
            // if there's a piece on the target square
            if (getBit(bitboards[bb_piece], toSq))
            {
                // remove it from corresponding bitboard
                target_piece = bb_piece;
                break;
            }
        }
               

        // score move by MVV LVA lookup [source piece][target piece]
        return mvv_lva[getMovePiece(move)][target_piece] + 10000;
    }


    // quiet promotions are also scored
    else if (getPromo(move))
    {
        return MoveScorePromoQuiet;
    }
   

    // score quiet move
    else
    {
        // score 1st killer move
        if (killers[0][ply] == move)
            return 9000;
        
        // score 2nd killer move
        else if (killers[1][ply] == move)
            return 8000;
        
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
static inline void enablePV_scoring(MoveList_t &MoveList)
{
    // disable following PV
    followPV = false;

    
    // loop over the moves within a move list
    for (int count = 0; count < MoveList.count; count++)
    {
        // make sure we hit PV move
        if (pv_table[0][ply] == MoveList.moves[count])
        {
            // enable move scoring and follow PV again
            scorePV  = true;
            followPV = true;
        }
    }
}



// getTimeInMilliseconds
//
// Get the number of milliseconds since epoch time.
static inline uint64_t getTimeInMilliseconds()
{
    return duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
}



// inputWaiting
//
// Function to "listen" to GUI's input during the search, without
// blocking the program. Credit goes to Richard Albert, author of
// VICE chess engine.
static inline int inputWaiting()
{
    #ifndef WIN64

        fd_set readfds;
        struct timeval tv;
        FD_ZERO (&readfds);
        FD_SET (fileno(stdin), &readfds);
        tv.tv_sec=0; tv.tv_usec=0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));


    #else

        static int init = 0, pipe;
        static HANDLE inh;
        DWORD dw;

        if (!init)
        {
            init = 1;
            inh = GetStdHandle(STD_INPUT_HANDLE);
            pipe = !GetConsoleMode(inh, &dw);
            if (!pipe)
            {
                SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
                FlushConsoleInputBuffer(inh);
            }
        }
        
        if (pipe)
        {
           if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
           return dw;
        }
        
        else
        {
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
static inline void watchClockAndInput()
{
    string cmd;

    
    while (!timedout)
    {
        // watch clock
        if (timeset && (getTimeInMilliseconds() > stoptime))
            timedout = true;


        // read the input
        else if (inputWaiting())
        {
            cin >> cmd;
            if (cmd == "stop")
                timedout = true;
        }

        // check for nodes limitation
        else if ((Limits.nodes > 0) && (nodes > Limits.nodes))
            timedout = true;


        // update interval 
        this_thread::sleep_for(chrono::milliseconds(WatchIntervalMs));
    }
}



// isEndgame
//
// Determine if the current position should be considered an endgame
// position for the current side to move.
static inline bool isEndgame()
{
    int pawn_material   = countBits(bitboards[P] | bitboards[p]) * 100;
    int knight_material = countBits(bitboards[N] | bitboards[n]) * 320;
    int bishop_material = countBits(bitboards[B] | bitboards[b]) * 320;
    int rook_material   = countBits(bitboards[R] | bitboards[r]) * 500;
    int queen_material  = countBits(bitboards[Q] | bitboards[q]) * 950;

	return ((pawn_material + knight_material + bishop_material
                           + rook_material + queen_material) < 2600);
}




// contempt
//
// Determine the draw score based on the phase of the game and whose moving,
// to encourge the engine to strive for a win in the middle-game, but be
// satisified with a draw in the endgame.
static inline int contempt()
{
    // in the endgame, it's ok to try to draw, if we're losing
    if (isEndgame())
        return DrawScore;


    // in the opening and middle game, we try to fight
    else
        return ((sideToMove == White) ? -Options["Contempt"] : Options["Contempt"]);
}



// mate_in
//
// Detect the distance from the root at which we can mate the opponent.
constexpr int mate_in(int ply)
{
    return MateValue - ply;
}




// mated_in
//
// Detect the distance from the root at which we are mated by our opponent.
constexpr int mated_in(int ply)
{
    return -MateValue + ply;
}



// futility_margin
//
// Calculate a futility margin based on a given depth.
constexpr int futility_margin(int depth)
{
    return 168 * depth;
}



// futility_move_count
//
// Calculate how far in the sorted list of moves can we start pruning.
constexpr int futility_move_count(int depth)
{
    return (3 + depth * depth) / 2;
}



#endif  //  SEARCH_H
