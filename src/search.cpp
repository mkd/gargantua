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
#include <iomanip>
#include <chrono>
#include <cassert>

#include "search.h"
#include "eval.h"



// 'nodes' is a global variable holding the number of nodes analyzed
// or searched. It is used by negamax() but also other performance test
// functions such as perft().
uint64_t nodes = 0ULL;



// Limits holds the configuration of the search: time, search depth, etc.
Limits_t Limits;



// Options holds more settings (aside from Limits) that can set via the
// setoption command in UCI.
std::map<string, int> Options;



// Time Control variables
uint64_t starttime = getTimeInMilliseconds();
uint64_t stoptime  = starttime;
uint64_t inc       = 0;
bool     timedout  = false;
bool     timeset   = true;



// killers [id][ply] 
//
// Killers is a table where the two best (quiet) moves are
// systematically stored for later searches. This is based on the
// fact that a move producing a beta cut-off must be a good one.
// beta cut-offs, where a move killer moves [id][ply]
//
// Note: storing exactly 2 killer moves is best for efficiency/performance.
int killers[2][MaxPly];



// history heuristics [piece][square]
int history[12][64];



// PV length [ply]
int pv_length[MaxPly];



// PV table [ply][ply]
int pv_table[MaxPly][MaxPly];



// follow PV & score PV move
bool followPV  = false;
bool scorePV   = false;



// allow Null move pruning
bool allowNull = true;



// Razoring and pruning margins
std::array<int, 4> LateMovePruningMargins = { 0, 8, 12, 24};
constexpr int RFPMargin = 64;
constexpr int RazorMargin = 339;



// Material value of pieces
std::array<int, 13> PieceValues = { 100, 300, 300, 500, 900, ValueInfinite,
                                    100, 300, 300, 500, 900, ValueInfinite, 0 };





// initSearch
//
// Intialize the search parameters to the default ones.
void initSearch()
{
    resetLimits();
}



// resetLimits
//
// Reset all search limits to their initial configuration.
void resetLimits()
{
    Limits.wtime     =  0;
    Limits.btime     =  0;
    Limits.winc      =  0;
    Limits.binc      =  0;
    Limits.npmsec    =  0;
    Limits.movetime  =  0;
    Limits.movestogo = 30;
    Limits.depth     = MaxSearchDepth;
    Limits.mate      =  0;
    Limits.perft     =  0;
    Limits.infinite  =  0;
    Limits.nodes     =  0;
    Limits.ponder    = false;
}



// negamax
//
// Main alphabeta algorithm (Negamax) which relies on a Principal Variation
// search.
//
int negamax(int alpha, int beta, int depth)
{
    // reliability checks
    assert(depth >= 0);


    // variables holding the calculatd score from negamax(), static evaluation
    // and margin for deciding whether to due forward pruning or not
    int score = 0, StaticEval = 0, EvalMargin = 0;


    // best move (to use with the transposition table)
    int bestmove = 0;


    // if the position is a draw, don't search anymore
    if (ply && isDraw())
        return contempt();


    // initialize hash flag for the transposition table
    int hash_type = hash_type_alpha;


    // figure out whether the current node is PV node or not
    bool pv_node = ((beta - alpha) > 1);


    // flag to detect whether we can prune futile moves
    bool canFutilityPrune = false;



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 1. Mate Distance Pruning (MDP) -- taken from Stockfish
    //
    // Even if we mate at the next move our score would be at best
    // mate_in(ply+1), but if alpha is already bigger because a
    // shorter mate was found upward in the tree then there is no need to
    // search because we will never beat the current alpha. Same logic but with
    // reversed signs applies also in the opposite condition of being mated
    // instead of giving mate. In this case return a fail-high score.
    //
    // @see https://www.chessprogramming.org/Mate_Distance_Pruning

    alpha = std::max(mated_in(ply), alpha);
    beta  = std::min(mate_in(ply+1), beta);
    if (alpha >= beta)
        return alpha;



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 2. Transposition Table (TT) lookup
    // 
    // Try to find the current node from the Transposition Table and return
    // the score immediately. We don't look in the hash table if we are in the
    // Principal Variation nodes.
    //
    // Note: in order to avoid artifacts in no-pawn endgames, we don't produce
    //       tranposition table cutoffs at very high rule50 counts (< 90).
    //
    // @see https://www.chessprogramming.org/Transposition_Table

    if (ply && ((score = TT::probe(alpha, beta, bestmove, depth)) != no_hash_found) && !pv_node)
        if (fifty < 90)
            return score;



    // If the Transposition Table did not return a hit, prepare for a more
    // comprehensive search by setting up the PV triangular array, resetting
    // the legal move counter to zero and increasing the nodes visited.
    //
    // Also, verify if the current position is in check to detect mate and
    // decide whether to search deeper (check extension).

    // init PV length
    pv_length[ply] = ply;

    // number of legal moves found
    int legal = 0;

    // increment nodes count
    nodes++;

    // is king in check? --> needed for detecting mate and in-check extension
    bool inCheck = isSquareAttacked((sideToMove == White) ? ls1b(bitboards[K]) : 
                                                            ls1b(bitboards[k]),
                                                            sideToMove ^ 1);



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 3. Check Extension
    // 
    // Extend the search depth by one if we're in check, so that we're less
    // likely to make a tactical mistake. I.e., don't call quiescence search
    // while in check.
    //
    // Also, if we are in check, we skip all reductions and jump directly into
    // the all-moves' PV search, without doing any forward pruning.
    //
    // @see https://www.chessprogramming.org/Check_Extensions

    if (inCheck)
    {
        depth++;
        goto moves_loop;
    }



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 4. Quiescence Search
    //
    // If we reach depth = 0, we are at a leaf node. Instead of returning a
    // static evaluation, go through all captures and promotions until the
    // position is stable enough; and then return its score.
    //
    // @see https://www.chessprogramming.org/Quiescence_Search

    if (depth == 0)
        return qsearch(alpha, beta);



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 5. Static Evaluation calculation
    //
    // In the following steps, we try to prune futile moves or estimate whether
    // a full search will be of any value. For this purpose, we need to know
    // what is the current static evaluation of the position. This will be then
    // used in conjunction with different margins and bonuses to check whether
    // we can fail low or high immediately without ending in the full search.

    StaticEval = evaluate();



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 6. Razoring
    //
    // If eval is really low, check with qsearch if it can exceed alpha, if it
    // can't, return a fail low.
    //
    // @see https://www.chessprogramming.org/Razoring

    if (ply && !pv_node
            && (depth < 2)
            && ((StaticEval + RazorMargin) <= alpha))
    {
        return qsearch(alpha, beta);
    }

    

    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 7. Reverse Futility Pruning (i.e., Static Null Pruning)
    // 
    // If our current material score is so good that even if we give
    // ourselves a big hit materially and subtract a large amount of our
    // material score (the "score margin") and our material score is still
    // greater than beta, we assume this node will fail-high and we can
    // prune its branch.
    //
    // @see https://www.chessprogramming.org/Reverse_Futility_Pruning

    if (!pv_node)
    {
        EvalMargin = depth * RFPMargin;
        if ((depth < 9) && (StaticEval - EvalMargin) >= beta)
                return (StaticEval - EvalMargin);
    }



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 8. Null move pruning
    //
	// If our opponent is given a free move, can they improve their position? If
    // we do a quick search after giving our opponent this free move and we
    // still find a move with a score better than beta, our opponent can't
    // improve their position and they wouldn't take this path, so we have a
    // beta cut-off and can prune  this branch.
    //
    // @see https://www.chessprogramming.org/Null_Move_Pruning

    if (!pv_node && allowNull && (depth >= 3) && !noMajorsOrMinors())
    {
        // R: is the reduction factor. The larger the R, the shallower the
        //    search is and the faster (but likely less reliable) the pruning
        //    estimate is.
        //
        // We use a dynamic reduction limit here (based on Blunder Chess Engine)
        // 
        // @see https://github.com/algerbrex/blunder/blob/main/engine/search.go
        int R = 3 + depth/6;

        // preserve board state
        saveBoard();
        
        // increment ply
        ply++;
        
        // increment repetition index & store hash key
        repetition_index++;
        repetition_table[repetition_index] = hash_key;
        
        // hash enpassant if available
        if (epsq != NoSq)
            hash_key ^= enpassant_keys[epsq];
        
        // reset enpassant capture square
        epsq = NoSq;
        
        // switch the side, literally giving opponent an extra move to make
        sideToMove ^= 1;
        
        // hash the side
        hash_key ^= side_key;

        // avoid doing 2 null moves in sequence
        allowNull = false;
                
        // search moves with reduced depth to find beta cutoffs
        score = -negamax(-beta, -beta + 1, depth - R - 1);

        // restore allowNull
        allowNull = true;


        // undo the null move
        repetition_index--;
        ply--;
        takeBack();


        // check if time is up
        if (timedout)
            return 0;


        // fail-hard beta cutoff
        if (score >= beta)
            return beta;
    }



    //////////////////////////////////////////////////////////////////////////
	// 
    // Step 9. Futility Pruning Detection (extended futility: beyond depth == 1)
    // 
    // If we're close to the horizon, and even with a large margin the static
    // evaluation can't be raised above alpha, we're probably in a fail-low
    // node, and many moves can be probably be pruned. So set a flag so we
    // don't waste time searching moves that suck and probably don't even have
    // a chance of raising alpha.
    //
    // @see https://www.chessprogramming.org/Futility_Pruning

    if (ply && !pv_node && (depth <= 8))
        if ((StaticEval + futility_margin(depth)) <= alpha)
			canFutilityPrune = true;



    //////////////////////////////////////////////////////////////////////////
	// 
    // Step 10. No-hashmove reduction (taken from Stockfish)
    //
    // If the position is not in TT, decrease depth by 1 (~3 Elo)
    if (pv_node && (depth >= 3) && !bestmove)
        depth--;



    // All-moves search begins here when in check, or after all forward
    // pruning techniques
    moves_loop:
 
        
    
    // create a new move list and generate the moves
    MoveList_t MoveList;
    generateMoves(MoveList);


    // if we are following PV line, enable PV move scoring
    if (followPV)
        enablePV_scoring(MoveList);


    // sort moves from best to worst
    sortMoves(MoveList, bestmove);


    // number of moves searched so far, within a move list
    int moves_searched = 0;

   

    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 11. Search all moves
    //
    // After doing all the early pruning, we jump into the main loop of going
    // through the moves available and search the score for each of them.

    for (int count = 0; count < MoveList.count; count++)
    {
        // preserve board state
        saveBoard();
       

        // increment ply
        ply++;


        // increment repetition index & store hash key
        repetition_index++;
        repetition_table[repetition_index] = hash_key;
       

        // make the move and check if it is illegal - skip it if so
        if (!makeMove(MoveList.moves[count]))
        {
            // in case of illegal move, undo it and skip to the next one
            repetition_index--;
            ply--;
            takeBack();
            
            continue;
        }


        // used for avoiding reductions on moves that give check
        bool givesCheck = isSquareAttacked((sideToMove == White) ? ls1b(bitboards[K]) : 
                                                                   ls1b(bitboards[k]),
                                                                   sideToMove ^ 1);


        // increment legal moves
        legal++;



        ////////////////////////////////////////////////////////////////////////
        //
        // Step 12. Full-width and full-depth search
        //
        // If this is the first move we are searching, we run a full search to
        // obtain a score that will guide the next searches.

        if (moves_searched == 0)
            score = -negamax(-beta, -alpha, depth - 1);



        else
        {
            ////////////////////////////////////////////////////////////////////////
            // 
            // Step 13. Futility Pruning on current move
            // 
            // If we're close to the horizon, and even with a large margin the static
            // evaluation can't be raised above alpha, we're probably in a fail-low
            // node, and many moves can be probably be pruned. So set a flag so we
            // don't waste time searching moves that suck and probably don't even have
            // a chance of raising alpha.
            //
            // @see https://www.chessprogramming.org/Futility_Pruning

            if (canFutilityPrune && (legal > 1))
            {
                if (!givesCheck && (killers[0][ply] != MoveList.moves[count])
                                && (killers[1][ply] != MoveList.moves[count])
                                && (getMovePiece(MoveList.moves[count]) != P)
                                && (getMovePiece(MoveList.moves[count]) != p)
                                && !getPromo(MoveList.moves[count])
                                && !getCastle(MoveList.moves[count])
                                && !getMoveCapture(MoveList.moves[count]))
                {
                    // undo the current move and skip to the next one
                    repetition_index--;
                    ply--;
                    takeBack();

                    continue;
                }
            }



            ////////////////////////////////////////////////////////////////////////
            // 
            // Step 14: Late Move Pruning (LMP)
            //
            // Because of move ordering, moves late in the move list are not very
            // likely to be interesting, so save time by completing pruning such
            // moves without searching them. Cauation needs to be taken we don't
            // miss a tactical move however, so the further away we prune from
            // the horizon, the "later" the move needs to be.

		    if (ply && !pv_node
                    && (depth <= 3)
                    && !inCheck
                    && !getMoveCapture(MoveList.moves[count])
                    && (legal > LateMovePruningMargins[depth]))
            {
                // undo the current move and skip to the next one
                repetition_index--;
                ply--;
                takeBack();

                continue;
			}



            ////////////////////////////////////////////////////////////////////
            //
            // Step 15. Late move reductions (LMR)
            //
            // Assuming that the moves in the list are ordered from potential
            // best to potential worst, analyzing the first moves is more
            // critical than the last ones. Therefore, using LMR we analyze the
            // first 3 moves in full-depth, but cut down the analysis depth for
            // the rest of moves.
            //
            // @see https://www.chessprogramming.org/Late_Move_Reductions

            if (ply && (legal >= LMRFullDepthMoves)
                    && (depth >= LMRReductionLimit)
                    && !inCheck
                    && !getMoveCapture(MoveList.moves[count]))
                score = -negamax(-alpha - 1, -alpha, depth - 2);

            
            // hack to ensure that full-depth search is done next
            else
                score = alpha + 1;
               


            ////////////////////////////////////////////////////////////////////
            //
            // Step 16. Principal Variation search (PVS)
            //
            // Once you've found a move with a score that is between alpha and
            // beta, the rest of the moves are searched with the goal of proving
            // that they are all bad. It's possible to do this a bit faster than
            // a search that worries that one of the remaining moves might be good.

            if (score > alpha)
            {
                score = -negamax(-alpha - 1, -alpha, depth - 1);
        

                // If the algorithm finds out that it was wrong, and that one of
                // the subsequent moves was better than the first PV move, it
                // has to search again, in the normal alpha-beta manner. This
                // happens sometimes, and it's a waste of time, but generally
                // not often enough to counteract the savings gained from doing
                // the "bad move proof" search referred to earlier.
                if ((score > alpha) && (score < beta))
                    score = -negamax(-beta, -alpha, depth - 1);
            }
        }


        
        // undo the move after the search
        repetition_index--;
        ply--;
        takeBack();



        // check if time is up
        if (timedout)
            return 0;



        // increment the counter of moves searched so far
        moves_searched++;



        // found a better move (improves alpha)
        if (score > alpha)
        {
            // hash entry type is PV node (exact score)
            hash_type = hash_type_exact;


            // store the best move in the TT
            bestmove = MoveList.moves[count];


            // store history moves (only for quiet moves)
            if (!getMoveCapture(MoveList.moves[count]))
                history[getMovePiece(MoveList.moves[count])][getMoveTarget(MoveList.moves[count])] += depth;


            // PV node (move)
            alpha = score;


            // write PV move
            pv_table[ply][ply] = MoveList.moves[count];

            
            // copy moves from deeper ply into current ply's line
            for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
                pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
           

            // adjust PV length
            pv_length[ply] = pv_length[ply + 1];            

        
            // fail-high (beta cutoff)
            if (score >= beta)
            {
                // store hash entry with the score equal to beta, only if not null move
                TT::save(beta, bestmove, depth, hash_type_beta);
               

                // store killer moves (only for quiet moves)
                if (!getMoveCapture(MoveList.moves[count]))
                {
                    killers[1][ply] = killers[0][ply];
                    killers[0][ply] = MoveList.moves[count];
                }


                // node (move) fails high
                return beta;
            }
        }
    }



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 17. Check if it's end of game
    //
    // If there are no legal moves, it's either checkmate or stalemate.

    if (legal == 0)
    {
        // king is in check: return mating score (closest distance to mate)
        if (inCheck)
            return -MateValue + ply;
        
        // king not in check: stalemate
        else
            //return DRAWSCORE;
            return contempt();
    }



    ///////////////////////////////////////////////////////////////////////////
    //
    // Step 18. Save the best move in the Transposition Table
    //
    // After finishing the search, we make sure we update the Transposition
    // Table with the best move.
    TT::save(alpha, bestmove, depth, hash_type);

   

    // if we reach here, we have failed low and return alpha as the best
    // possible score
    return alpha;
}



// search
//
// The search is started when the program receives the UCI 'go'
// command. It searches from the root position and outputs the "bestmove".
void search()
{
    // reliability checks
    assert(Limits.depth >= 0);


    // set the flag for the engine to know the search is started
    timedout = false;


    // define best score
    int score;


    // start the timer as soon as possible
    auto start = chrono::high_resolution_clock::now();


    // reset data structures for a new search
    memset(killers, 0, sizeof(killers));
    memset(history, 0, sizeof(history));
    memset(pv_table, 0, sizeof(pv_table));
    memset(pv_length, 0, sizeof(pv_length));


    // reset follow PV flags
    followPV   = false;
    scorePV    = false;
    allowNull  = true;


    // define initial alpha beta bounds
    int alpha = -ValueInfinite;
    int beta  =  ValueInfinite;


    // reset nodes counter
    nodes = 0ULL;


    // reset "time is up" flag
    timedout = false;


    // iterative deepening framework
    for (int current_depth = 1; current_depth <= Limits.depth; current_depth++)
    {
        // stop calculating and return best move so far 
        if (timedout)
            break;


        // enable follow PV flag
        followPV = true;
   

        // find best move within a given position
        score = negamax(alpha, beta, current_depth);



        ////////////////////////////////////////////////////////////////////////
        //
        // Aspiration Window
        //
        // Search with a narrow window, keep narrowing it after each iteration.
        // However, if the score falls outside the window, we must try again
        // with a full-width window (and the same depth).

        if ((score <= alpha) || (score >= beta))
        {
            alpha = -ValueInfinite;
            beta  =  ValueInfinite;
            current_depth--;
            continue;
        }


        // set up the window for the next iteration
        alpha = score - AspirationWindow;
        beta  = score + AspirationWindow;


        // stop the timer and measure time elapsed
        auto finish = chrono::high_resolution_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(finish-start).count();
        auto ns = chrono::duration_cast<chrono::nanoseconds>(finish-start).count();
        
    
        // print bestmove and PV line
        if (pv_length[0])
        {
            cout << "info depth " << current_depth;

            // report mating distance if available, otherwise print score
            if ((score > -MateValue) && (score < -MateScore))
                cout << " score mate " << -(score + MateValue) / 2 - 1;
            else if ((score > MateScore) && (score < MateValue))
                cout << " score mate " << (MateValue - score) / 2 + 1;
            else
                cout << " score cp " << score;

            // other search information: nodes, nps, time, etc.
            cout << " nodes " <<  nodes
                 << " nps " << nodes * 1000000000 / ns
                 << " hashfull " << TT::hashfull()
                 << " time " << ms
                 << " pv ";
            
            // print PV line
            for (int count = 0; count < pv_length[0]; count++)
                cout << prettyMove(pv_table[0][count]) << " ";


            // new line before next depth
            cout << endl << flush;
        }
    }


    // print bestmove
    cout << "bestmove " << prettyMove(pv_table[0][0]) << endl << flush;


    // tell the engine that the search is ready
    timedout = true;
}



// qsearch
//
// Called by the main search, this is a function with zero depth. qsearch()
// performs an indefinite search until all the following conditions are met:
//
// a) no more possible captures
// b) no more pawn promotions
// c) depth is too deep or time (from a running timer) is up
int qsearch(int alpha, int beta)
{
    // start searching a score from the beginning (= -ValueInfinite)
    int val, score;


    // increment nodes count
    nodes++;


    // we are too deep, hence there's an overflow of arrays relying on max ply constant
    if (ply > (MaxPly - 1))
        return evaluate();


    // calculate "stand-pat" to stabilize the qsearch
    val = evaluate();


    // beta-cutoff
    if (val >= beta)
        return beta;

    
    // found a better move (PV node position)
    if (val > alpha)
        alpha = val;
   

    // generate a new move list and sort it
    MoveList_t MoveList;
    generateCapturesAndPromotions(MoveList);
    sortMoves(MoveList, 0);

    
    // loop over moves within a movelist
    for (int count = 0; count < MoveList.count; count++)
    {
        // don't search capture sequences that end up in losing material
        if (see(MoveList.moves[count]) < 0)
            continue;


        // preserve board state
        saveBoard();
       

        // increment ply
        ply++;

        
        // increment repetition index & store hash key
        repetition_index++;
        repetition_table[repetition_index] = hash_key;

        
        // make sure to make only legal moves
        if (!makeMove(MoveList.moves[count]))
        {
            // in case of illegal move, undo it and skip to the next one
            repetition_index--;
            ply--;
            takeBack();
            
            continue;
        }


        // score current move
        score = -qsearch(-beta, -alpha);
       

        // undo the move after we got its score
        repetition_index--;
        ply--;
        takeBack();


        // check if time is up
        if (timedout)
            return 0;

       
        // found a better move
        if (score > alpha)
        {
            // PV node (position)
            alpha = score;


            // fail-high beta cutoff
            if (score >= beta)
                return beta;
        }
    }
   

    // node (position) fails low
    return alpha;
}



// dperft
//
// Divide-perft is a perft() wrapper that divides a position into each
// root move and calls perft() for each of them. This is very useful
// to debug possible errors within the move generator for a given root move.
void dperft(int depth)
{
    // reliability checks
    assert(depth > 0);


    // reset nodes count
    nodes = 0ULL;
   

    // create move list instance
    MoveList_t MoveList;

    
    // generate moves
    generateMoves(MoveList);

    
    // init start time
    auto start = chrono::high_resolution_clock::now();
   

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


        // cummulative nodes
        uint64_t cNodes = nodes;


        // call perft driver recursively
        perft(depth - 1);

        
        // old nodes
        uint64_t PrevNodes = nodes - cNodes;

        
        // undo move
        takeBack();


        // print move and nodes under that move
        cout << prettyMove(MoveList.moves[move_count]) << ": " << PrevNodes << endl;
    }


    // stop the timer and measure time elapsed
    auto finish = chrono::high_resolution_clock::now();
    auto ns = chrono::duration_cast<chrono::nanoseconds>(finish-start).count();
    assert(ns);


    // print results
    cout << endl;
    cout << "    Depth: " << depth << endl;
    cout << "    Nodes: " << nodes << endl;
    cout << fixed << setprecision(3);
    cout << "    Time:  " << ns / 1000000.0 << "ms" << endl;
    cout << "   Speed:  " << nodes * 1000000 / ns << " Knps" << endl << endl;
}



// sortMoves
//
// Take a MoveList_t and sort it in descending order by move score.
//
// sortMoves() works in a way that goes through every move in the move
// list, gets a score for every move (based on MVV/LVA, PV node, and
// other heuristics), and sorts the list from bigger score to smaller 
// score. In other words, sortMoves() tries to bring the best move to
// the front.
//
// If a "best move" is found in the transposition table, it is placed
// at the top, making it the first move to be searched.
void sortMoves(MoveList_t &MoveList, int bestmove)
{
    // reliability checks
    assert(MoveList.count > 0);
    assert(MoveList.count < 256);


    // associative array linking moves and scores
    pair<int, int> pairt[256];


    // find the score for every move
    for (int i = 0; i < MoveList.count; i++) 
    {
        // score bestmove (from TT) above all
        if (MoveList.moves[i] == bestmove)
            pairt[i].first = 30000;

        // rest of moves are scored using scoreMove()
        else
            pairt[i].first  = scoreMove(MoveList.moves[i]);

        // associate score with the move
        pairt[i].second = MoveList.moves[i];
    }
  
    // sort the associative array based on the move score
    sort(pairt, pairt + MoveList.count, greater<pair<int, int>>());
     

    // re-write the MoveList ordered by move score in descending order
    for (int i = 0; i < MoveList.count; i++) 
        MoveList.moves[i] = pairt[i].second;
}



// printMoveScores
//
// This function is for testing move scoring and ordering.
void printMoveScores(MoveList_t &MoveList)
{
    cout << "     Move scores:" << endl << endl;
       

    // loop over moves within a move list
    for (int count = 0; count < MoveList.count; count++)
    {
        cout << "     move: ";
        cout << prettyMove(MoveList.moves[count]);
        cout << " score: " << scoreMove(MoveList.moves[count]) << endl;
    }
    cout << endl << endl;
}



// resetTimeControl
//
// Set the internal time configuration back to the default one. This is
// typically done when parsing a new 'go' command from UCI.
void resetTimeControl()
{
    // reset timing
    inc       = 0;
    stoptime  = 0;
    timeset   = 1;
    timedout  = false; 

    starttime = getTimeInMilliseconds();
    Limits.movestogo = 30;
    Limits.movetime  =  0;
}



// see
//
// Simulate a capturing sequence on the target square of the move given
// (static exchange evaluation), and return the final score of the move 
// (after completing all the captures) from the perspective of the side
// to move.
int see(int move)
{
    // total static evaluation after all possible exchanges have been made
    std::array<int, 32> gain;


    // internal depth keeps track of which 'ply' within the capture simulation
    // we are
    int idepth = 0;


    // change side to move
    int stm = sideToMove ^ 1;


    // initialize origin and target square, as well as attacker and target piece
    int fromSq   = getMoveSource(move);
    int toSq     = getMoveTarget(move);
    int attacker = getMovePiece(move);

    
    // identify the piece on the target square
    int start_piece = P, end_piece = K, target = -1;
    
    if (stm == Black)
    {
        start_piece = p;
        end_piece = k;
    }

    for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
    {
        if (getBit(bitboards[bb_piece], toSq))
        {
            target = bb_piece;
            break;
        }
    }


    // if no piece to capture at target, this is not a capture
    if (target < 0)
        return 0;


    // temporary bitboards to run the exchange simulation
    Bitboard seenBB     = 0ULL;
	Bitboard occupiedBB = occupancies[White] | occupancies[Black];
	Bitboard attackerBB = SqBB[fromSq];


    // list of attackers and defenders to a given square
    Bitboard attadef = getAttackers(White, toSq, occupiedBB) | getAttackers(Black, toSq, occupiedBB);
    Bitboard maxXray = occupiedBB & ~(bitboards[N] | bitboards[K] | bitboards[n] | bitboards[k]);


    // calcualte initial win, from the first capture
    gain[idepth] = PieceValues[target];


    // simulate the whole capturing sequence and calculate the total gain
    while (attackerBB)
    {
        idepth++;
        gain[idepth] = PieceValues[attacker] - gain[idepth - 1];

        if (std::max(-gain[idepth-1], gain[idepth]) < 0)
            break;

        attadef &= ~attackerBB;
        occupiedBB &= ~attackerBB;
        seenBB |= attackerBB;

        if ((attackerBB & maxXray) != 0)
            attadef |= considerXrays(toSq, occupiedBB) & ~seenBB;

        attackerBB = minAttacker(attadef, stm, attacker);
        stm ^= 1;
    }


    // calculate the total net score after completing all the captures
    for (idepth--; idepth > 0; idepth--)
        gain[idepth-1] = -std::max(-gain[idepth-1], gain[idepth]);


    // return the total net score of the capturing sequence
    return gain[0];
}
