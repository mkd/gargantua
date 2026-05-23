# Gargantua UCI Chess Engine

## Overview
Gargantua is a UCI chess engine with [NNUE](https://www.chessprogramming.org/NNUE)
evaluation, derived from [Chess0](https://github.com/mkd/chess0), and inspired
by Code Monkey King's [BBC](https://github.com/maksimKorzh/bbc).

Gargantua's current approximate ELO rating is around ~3350, based on the
[Elometer test](http://www.elometer.net), various tournaments against other
rated chess engines (via Cutechess), and a series of online games played on the
[Free Internet Chess Server (FICS)](https://www.freechess.org/).

## Implementation
Gargantua is implemented mainly in C++, leveraging the STL as much as possible.
However, there are many routines and data structures that are written in
pure C language for performance reasons.

Gargantua relies on Stockfish' neural networks (NNUE) to evaluate the score
of a given position. This is based on 
[Daniel Shawul's NNUE-probe implementation](https://github.com/dshawul/nnue-probe).


In order to ensure a proper behavior, you need to make sure the neural network
file (i.e., nn-eba324f53044.nnue) is in the current directory of execution. This
also means you must configure the path to Gargantua in your GUI, so that it can
find the neural network evaluation file. Otherwise, the engine will play random
moves.


## Search features and optimizations
- **Bitboards:** 
  https://en.wikipedia.org/wiki/Bitboard

- **Universal Chess Interface (UCI) protocol:**
  http://wbec-ridderkerk.nl/html/UCIProtocol.html

- **NNUE evaluation function:** an evaluation function based on a neural
  network trained with millions of games played by Stockfish 11 at a
  moderate depth. More here: https://www.chessprogramming.org/NNUE

- **Aspiration Windows (Advanced):**
  Uses dynamic widening margins to quickly prove fail-highs and fail-lows near the root.
  https://www.chessprogramming.org/Aspiration_Windows

- **Mate Distance Pruning (MDP):**
  https://www.chessprogramming.org/Mate_Distance_Pruning

- **Transposition Table (TT):**
  https://en.wikipedia.org/wiki/Transposition_table

- **Check Extension:**

- **Singular Extensions:**
  Explores forced tactical variations deeper by extending search depth if a TT move is strictly better than all alternatives by a calculated margin.
  https://www.chessprogramming.org/Singular_Extension

- **Quiescence search:**
  https://en.wikipedia.org/wiki/Quiescence_search

- **Late Move Reductions (Logarithmic LMR):** 
  Reduces search depth for late quiet moves using a `ln(depth) * ln(moves)` formula, optimized with history scores.
  https://www.chessprogramming.org/Late_Move_Reductions

- **Move Ordering Heuristics:**
  - **MVV-LVA (Most Valuable Victim - Least Valuable Attacker)**
  - **Countermove Heuristic:** Uses the previous move to index and prioritize specific response moves.
  - **Killer Move Heuristic:** Prioritizes moves that caused beta cutoffs in sibling nodes at the same ply.
  - **History Gravity:** Dynamically rewards quiet moves causing cutoffs with a depth-squared bonus globally, penalizing those that fail to cause cutoffs.

- **Razoring:**
  https://www.chessprogramming.org/Razoring

- **Futility Pruning**
  https://www.chessprogramming.org/Futility_Pruning

- **Reverse Futility Pruning (i.e., Static Null Move Pruning):**
  https://www.chessprogramming.org/Reverse_Futility_Pruning

- **Principal Variation Search:** 
  https://en.wikipedia.org/wiki/Principal_variation_search

- **Iterative Deepening:**:
  https://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search

- **Alpha-beta pruning:**
  https://en.wikipedia.org/wiki/Alpha–beta_pruning
 
- **Null Move Reductions:** 
  https://en.wikipedia.org/wiki/Null-move_heuristic

- **Lazy SMP (Multi-Threading):** 
  Utilizes a lockless shared Transposition Table for highly scalable and efficient multi-threading.


## Key Features in v2+ versions
- **New NNUE Architecture**: Shifted to a robust Stockfish-based probe implementation (`src/stockfish_probe`) for state-of-the-art evaluation.
- **Lazy SMP**: Fully thread-safe engine supporting highly scalable multi-threaded searches.
- **Syzygy Tablebase Support**: precise endgame play with support for 3-4-5-6-7 piece tablebases.
- **Crash Recovery System**: Implemented a dedicated signal handling mechanism to ensure maximum stability even during intensive tablebase probes.
- **Simplified Build**: A clean and standardized build process.
- **Lazy SMP (Multi-Threading)**


## Files
- [src](src): Source code (C++17).
- [nn-b1a57edbea57.nnue](src/nn-b1a57edbea57.nnue): Primary neural network.
- [tests](tests): Python-based testing framework.


## Building Gargantua
To build the engine, simply run:
```bash
cd src
make -j ARCH=apple-silicon
```
This produces the `gargantua` binary.


## Using Gargantua
Thanks to the [UCI protocol](http://wbec-ridderkerk.nl/html/UCIProtocol.html),
you can use Gargantua with your favorite chess GUI.
Chess GUI applications use the UCI protocol to communicate with the engine, so
you can analyze your games, play against it or even run a computer chess
tournament from your own computer.

I personally use [ScidvsMac](http://scidvspc.sourceforge.net/) (a version
of ScidvsPC for Mac) to test Gargantua. There's a ton of free chess graphical
interfaces for UCI chess engines, just pick your favorite one.

Gargantua also has a few built-in commands that are not officially part of the
UCI protocol. These commands are used for debugging or testing. Here's a short 
list:
- d: display the current position on the chess board
- eval: show the NNUE static evaluation of the current position
- flip: flip the view of the chess board when printing a position
- moves: print a list of all pseudo-legal moves
- smoves: print the list of available moves, sorted from best to worst


### UCI Options
- **Threads**: Number of threads for the engine to use during search (default 1).
- **Hash**: Size of the transposition table in MB.
- **SyzygyPath**: Absolute path to your Syzygy tablebase files (`.rtbw`, `.rtbz`).
  - Example: `SyzygyPath=/path/to/syzygy/3-4-5`


## Testing Framework
Gargantua includes a powerful testing framework in `tests/manager.py` for automated matches and Elo measurement using `cutechess-cli`.

Example: Run a 100-game match against a previous version:
```bash
python3 tests/manager.py --match --base ./gargantua-v1 --games 100 --tc 10+0.1
```


# Contributing to Gargantua
My work on Gargantua is very sporadic and not a priority in my life, so the
development is not very active. So, if you have tried Gargantua and would like
to improve the current source code in any way, please go ahead!


## Credits
- **Claudio M. Camacho**: Author
- **Stockfish Team**: For the NNUE and Tablebase probe implementation.
- **Daniel Shawul**: For the original NNUE-probe inspiration.
