# Overview
Gargantua is a UCI chess engine with [NNUE](https://www.chessprogramming.org/NNUE)
evaluation, derived from [Chess0](https://github.com/mkd/chess0), and inspired
by Code Monkey King's [BBC](https://github.com/maksimKorzh/bbc).

Gargantua's current approximate ELO rating is around ~3100, based on the
[Elometer test](http://www.elometer.net), various tournaments against other
rated chess engines, and a series of online games played on the
[Free Internet Chess Server (FICS)](https://www.freechess.org/).



# Files
This distribution of Gargantua consists of the following files:
- [README.md](https://github.com/mkd/gargantua/blob/master/README.md), the file
  you are currently reading.
- [copying.txt](https://github.com/mkd/gargantua/blob/master/copying.txt), a
  text file containing the GNU General Public License version 3.
- [TODO](https://github.com/mkd/gargantua/blob/master/TODO), a list of
  improvements and future features (i.e., a backlog).
- [Changelog](https://github.com/mkd/gargantua/blob/master/Changelog), a readable
  list of changes introduced in every version.
- [src](https://github.com/mkd/gargantua/blob/master/src), a subdirectory
  containing the full source code, including a Makefile that can be used
  to compile Gargantua.
- [src/nn-eba324f53044.nnue](https://github.com/mkd/gargantua/blob/master/src/nn-eba324f53044.nnue),
  a neural network from Stockfish used for heuristic evaluation purposes.
- [src/benchmark.py](https://github.com/mkd/gargantua/blob/master/src/benchmark.py),
  a simple script written in Python3 that tests many different chess positions
  using the python-chess UCI library, telling the speed of search, nodes and
  time to complete the tests. The script is engine-agnostic, and it can be 
  used to benchmark any UCI chess engine.



# Building Gargantua
The default [Makefile](https://github.com/gargantua/blob/master/src/Makefile)
in [src](https://github.com/mkd/gargantua/blob/master/src) contains the
instructions to build Gargantua, with build targets for macOS, GNU/Linux,
Unix systems in general, as well as Windows 64-bit.



# Using Gargantua
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



# Implementation
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



# Contributing to Gargantua
My work on Gargantua is very sporadic and not a priority in my life, so the
development is not very active. So, if you have tried Gargantua and would like
to improve the current source code in any way, please go ahead!



# Search features and optimizations
- **Bitboards:** 
  https://en.wikipedia.org/wiki/Bitboard

- **Universal Chess Interface (UCI) protocol:**
  http://wbec-ridderkerk.nl/html/UCIProtocol.html

- **NNUE evaluation function:** an evaluation function based on a neural
  network trained with millions of games played by Stockfish 11 at a
  moderate depth. More here: https://www.chessprogramming.org/NNUE

- **Aspiration Windows:**
  https://www.chessprogramming.org/Aspiration_Windows

- **Mate Distance Pruning (MDP):**
  https://www.chessprogramming.org/Mate_Distance_Pruning

- **Transposition Table (TT):**
  https://en.wikipedia.org/wiki/Transposition_table

- **Check Extension:**

- **Quiescence search:**
  https://en.wikipedia.org/wiki/Quiescence_search

- **Late Move Reductions (LMR):** 
  https://www.chessprogramming.org/Late_Move_Reductions

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



# Testing new features
Every time a new feature is added to the codebase, with the goal of improving 
Gargantua's playing strength, it needs to be tested against the latest stable
version (master branch). The goal of the test is to see whether the improvement
makes the engine stronger or weaker at gameplay.

Testing a new improvement consists of three steps:
1. Run benchmark.py to test the search speed and time-to-depth
2. Run benchmark.py to see whether the number of nodes searched drops
3. Play 300 games with TC 5min per side (0 increment); using a varied
   opening book (gm2600.bin), for opening variability, that comes pre-installed
   in Scid vs. Mac.

In some cases, the number of nodes searched and the search speed might become
worse than in the master version, yet the play quality might increase
considerably (e.g., when adding a search extension that improves gameplay).
In this case, the version that plays better (winner of the games) is chosen
over the other one.
