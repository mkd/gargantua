#ifndef NNUE_INCREMENTAL_H
#define NNUE_INCREMENTAL_H

#include "position.h"

namespace Stockfish {
namespace Incremental {

// Initialize the global Stockfish position from a FEN string/startpos
void init();

// Perform a move incrementally on the internal Stockfish position.
void do_move(int g_move, StateInfo *si);

// Perform a null move (just update state linkage and side).
void do_null_move(StateInfo *si);

// Undo the last move (restore previous state).
void undo_move(int g_move);

// Safe setup functions for UCI::position (Persistent StateInfo management)
void setup_reset(const std::string &fen);
void setup_move(int g_move);
void setup_undo(int g_move);

// Stack-based Incremental Move Handling (Avoids Heap Allocation)
void push_state(int g_move);
void push_null_state();
void pop_state(int g_move);

// Get the evaluation of the current position.
int evaluate();

// Thread synchronization
const Position& get_global_pos();
const std::deque<StateInfo>& get_setup_states();
void sync_from_main_thread(const Position& main_pos, const std::deque<StateInfo>& main_setup);

} // namespace Incremental
} // namespace Stockfish

#endif
