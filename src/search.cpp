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
int killers[2][MAXPLY];



// history heuristics [piece][square]
int history[12][64];



// PV length [ply]
int pv_length[MAXPLY];



// PV table [ply][ply]
int pv_table[MAXPLY][MAXPLY];



// follow PV & score PV move
bool followPV  = false;
bool scorePV   = false;



// allow Null move pruning
bool allowNull = true;



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
    Limits.wtime     = 0;
    Limits.btime     = 0;
    Limits.winc      = 0;
    Limits.binc      = 0;
    Limits.npmsec    = 0;
    Limits.movetime  = 0;
    Limits.movestogo = 40;
    Limits.depth     = MAX_SEARCH_DEPTH;
    Limits.mate      = 0;
    Limits.perft     = 0;
    Limits.infinite  = 0;
    Limits.nodes     = 0;
    Limits.ponder    = false;
}



// negamax
//
// Main alphabeta algorithm (Negamax) which relies on a Principal Variation
// search. This algorithm uses the following steps:
//
//  1. static null move pruning
//  2. null move pruning
//  3. razoring
//  2. sort moves (score based on historic appearance, capture gain, etc)
//  3. look up move hash (from previous searches)
//  4. late move reductions (LMR)
//  8. start full search
//
// The score returned by the algorithm is always from calling qsearch().
int negamax(int alpha, int beta, int depth)
{
    // reliability checks
    assert(depth >= 0);


    // variable holding the calculatd score from negamax()
    int score;


    // best move (to use with the transposition table)
    int bestmove = 0;


    // if the position is a draw, don't search anymore
    if (ply && isDraw())
        //return DRAWSCORE;
        return contempt();


    // initialize hash flag for the transposition table
    int hash_type = hash_type_alpha;


    // figure out whether the current node is PV node or not
    bool pv_node = ((beta - alpha) > 1);


    // Step 1. Transposition Table (TT)
    // 
    // Try to find the current node from the Transposition Table and return
    // the score immediately.
    if (ply && ((score = TT::probe(alpha, beta, bestmove, depth)) != no_hash_found) && !pv_node)
        return score;


    // init PV length
    pv_length[ply] = ply;


    // increment nodes count
    nodes++;


    // is king in check? --> needed for dectecting mate and in-check extension
    bool inCheck = isSquareAttacked((sideToMove == White) ? ls1b(bitboards[K]) : 
                                                            ls1b(bitboards[k]),
                                                            sideToMove ^ 1);


    // Extend the search depth by one if we're in check, so that we're less
    // likely to make a tactical mistake. I.e., don't call quiescence search
    // while in check.
    if (inCheck)
        depth++;


    // leaf node: return static evaluation
    if (depth == 0)
        return qsearch(alpha, beta);


    // number of legal moves found
    int legal = 0;


    // Step 2. Static Null Move Pruning
    // 
    // If our current material score is so good that even if we give
    // ourselves a big hit materially and subtract a large amount of our
    // material score (the "score margin") and our material score is still
    // greater than beta, we assume this node will fail-high and we can
    // prune its branch.   
	int static_eval = evaluate();
    
    // evaluation pruning / static null move pruning
	if ((depth < 3) && !pv_node
                    && !inCheck
                    && (abs(beta - 1) > -VALUE_INFINITE + 100))
	{   
        // define evaluation margin
		int eval_margin = STATIC_NULLMOVE_PRUNING_MARGIN * depth;
		
		// evaluation margin substracted from static evaluation score
		if (static_eval - eval_margin >= beta)
			return beta;
	}


    // Step 3. Null Move Pruning
    //
	// If our opponet is given a free move, can they improve their position? If
    // we do a quick search after giving our opponet this free move and we still
    // find a move with a score better than beta, our opponet can't improve
    // their position and they wouldn't take this path, so we have a beta
    // cut-off and can prune  this branch.                      
    //
    //
    if (allowNull && (depth >= 3)
                  && !inCheck
                  && !pv_node
                  && !noMajorsOrMinors())
    {
        // R: is the reduction factor. The larger the R, the shallower the
        //    search is and the faster (but likely less reliable) the pruning
        //    estimate is.
        //
        // We use a dynamic reduction limit here (based on Blunder Chess Engine).
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
        score = -negamax(-beta, -beta + 1, depth - R);

        // restore allowNull
        allowNull = true;

        // decrement ply
        ply--;

        // decrement repetition index
        repetition_index--;

        // restore board state
        takeBack();


        // reutrn 0 if time is up
        if (timedout)
            return 0;


        // fail-hard beta cutoff
        if (score >= beta)
            return beta;
    }


    // Step 4. Razoring
    //
    // If eval is really low check with qsearch if it can exceed alpha, if it
    // can't, return a fail low.

    /*
    if (!pv_node && (depth <= 7) 
                 && (static_eval < alpha - 348 - 258 * depth * depth))
    {
        int new_score = qsearch(alpha - 1, alpha);
        if (new_score < alpha)
            return new_score;
    }
    */
    
    if (!pv_node && !inCheck && (depth <= 3))
    {
        // get static eval and add first bonus
        score = static_eval + 125;
        
        // define new score
        int new_score;
        
        // static evaluation indicates a fail-low node
        if (score < beta)
        {
            // on depth 1
            if (depth == 1)
            {
                // get quiscence score
                new_score = qsearch(alpha, beta);
                
                // return quiescence score if it's greater then static evaluation score
                return (new_score > score) ? new_score : score;
            }
            
            // add second bonus to static evaluation
            score += 175;
            
            // static evaluation indicates a fail-low node
            if (score < beta && depth <= 2)
            {
                // get quiscence score
                new_score = qsearch(alpha, beta);
                
                // quiescence score indicates fail-low node
                if (new_score < beta)
                    // return quiescence score if it's greater then static evaluation score
                    return (new_score > score) ? new_score : score;
            }
        }
	}

    
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

    
    // loop over moves and search the score for each move
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
            // decrement ply
            ply--;

            // decrement repetition index
            repetition_index--;

            // undo move
            takeBack();
            
            // skip to the next move
            continue;
        }


        // increment legal moves
        legal++;


        // Step 4. Full-width and full-depth search, if no moves searched yet
        if (moves_searched == 0)
            score = -negamax(-beta, -alpha, depth - 1);

        
        // Step 5. Late Move Reductions (LMR)
        //
        // Configure late-move reductions (LMR): assuming that the moves in the
        // list are ordered from potential best to potential worst, analyzing 
        // the first moves is more critical than the last ones. Therefore, 
        // using LMR we analyze the first 3 moves in full-depth, but cut down
        // the analysis depth for the rest of moves.
        else
        {
            if(
                moves_searched >= LMR_FULLDEPTH_MOVES &&
                depth >= LMR_REDUCTION_LIMIT &&
                !inCheck && 
                !getMoveCapture(MoveList.moves[count]) &&
                !getPromo(MoveList.moves[count])
              )
                score = -negamax(-alpha - 1, -alpha, depth - 2);

            
            // hack to ensure that full-depth search is done next
            else
                score = alpha + 1;
               

            // Step 6. Principal Variation Search
            if (score > alpha)
            {
                // Once you've found a move with a score that is between alpha and beta,
                // the rest of the moves are searched with the goal of proving that they are all bad.
                // It's possible to do this a bit faster than a search that worries that one
                // of the remaining moves might be good. */
                score = -negamax(-alpha - 1, -alpha, depth - 1);
        

                // If the algorithm finds out that it was wrong, and that one of the
                // subsequent moves was better than the first PV move, it has to search again,
                // in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                // but generally not often enough to counteract the savings gained from doing the
                // "bad move proof" search referred to earlier.
                if ((score > alpha) && (score < beta))
                    score = -negamax(-beta, -alpha, depth - 1);
            }
        }

        
        // decrement ply
        ply--;


        // decrement repetition index
        repetition_index--;


        // undo move
        takeBack();


        // reutrn 0 if time is up
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
                //if (bestmove)
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


    // checkmate or stalemate detection
    if (legal == 0)
    {
        // king is in check: return mating score (closest distance to mate)
        if (inCheck)
            return -MATEVALUE + ply;
        
        // king not in check: stalemate
        else
            //return DRAWSCORE;
            return contempt();
    }


    // store hash entry with the score equal to alpha, only if not null move
    //if (bestmove)
    TT::save(alpha, bestmove, depth, hash_type);

    
    // node (move) fails low
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
    int alpha = -VALUE_INFINITE;
    int beta  =  VALUE_INFINITE;


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


        // Aspiration Window: search with a narrow window, keep narrowing it
        // after each iteration. However, if the score falls outside the window,
        // we must try again with a full-width window (and the same depth).
        if ((score <= alpha) || (score >= beta))
        {
            alpha = -VALUE_INFINITE;
            beta  =  VALUE_INFINITE;
            current_depth--;
            continue;
        }

        
        // set up the window for the next iteration
        alpha = score - ASPIRATION_WINDOW_SIZE;
        beta  = score + ASPIRATION_WINDOW_SIZE;


        // stop the timer and measure time elapsed
        auto finish = chrono::high_resolution_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(finish-start).count();
        auto ns = chrono::duration_cast<chrono::nanoseconds>(finish-start).count();
        
    
        // print bestmove and PV line
        if (pv_length[0])
        {
            cout << "info depth " << current_depth;

            // report mating distance if available, otherwise print score
            if ((score > -MATEVALUE) && (score < -MATESCORE))
                cout << " score mate " << -(score + MATEVALUE) / 2 - 1;
            else if ((score > MATESCORE) && (score < MATEVALUE))
                cout << " score mate " << (MATEVALUE - score) / 2 + 1;
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
    // start searching a score from the beginning (= -VALUE_INFINITE)
    int val, score;


    // increment nodes count
    nodes++;


    // we are too deep, hence there's an overflow of arrays relying on max ply constant
    if (ply > (MAXPLY - 1))
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
            // decrement ply
            ply--;
            
            // decrement repetition index
            repetition_index--;

            // undo move
            takeBack();
            
            // skip to next move
            continue;
        }


        // score current move
        score = -qsearch(-beta, -alpha);
       

        // decrement ply
        ply--;

        
        // decrement repetition index
        repetition_index--;


        // take move back
        takeBack();


        // reutrn 0 if time is up
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
    //assert(MoveList.count > 0);
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
    Limits.movestogo = 40;
    Limits.movetime  =  0;
}
