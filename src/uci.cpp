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

#include <string>

#include "bitboard.h"
#include "movgen.h"
#include "position.h"
#include "search.h"
#include "uci.h"



using namespace std;



// UCI::moveToString converts a Move to a string in coordinate notation
// (e.g., g1f3, a7a8q).
//
// The only special case is castling, where we print in the e1g1 notation in
// normal chess mode, and in e1h1 notation in chess960 mode. Internally all
// castling moves are always encoded as 'king captures rook'.
string UCI::moveToString(int m)
{
    int fromSq = getMoveSource(m);
    int toSq   = getMoveTarget(m);


    //if (m == MOVE_NULL)
    //    return "0000";


    // castling
    //if (getCastle(m))
    //    toSq = make_square(to > from ? FILE_G : FILE_C, rank_of(from));


    // compose move in UCI mode: origin square + target square (e.g., e2e4)
    string move = SquareToCoordinates[fromSq] + SquareToCoordinates[toSq];


    // add promotion piece
    if (getPromo(m))
        move += PromoPieces[getPromo(m)];


    // return move string
    return move;
}



// UCI::parseMove
//
// UCI::to_move() converts a string representing a move in coordinate notation
// (g1f3, a7a8q) to the corresponding (pseudo) legal move, if any.
//
// Note: the move will be considered legal if it is in the pseudo-legal moves
//       list. That means you must take care of checking legality after parsing
//       the move, before making it on the board; i.e, is King in check?
int UCI::parseMove(string str)
{
    // verify promotion and make sure it is in lower-case
    if (str.length() == 5)
        str[4] = char(tolower(str[4]));


    // generate all moves
    MoveList_t MoveList;
    generateMoves(MoveList);


    // try to find the move in the list of pseudo-legal moves
    for (int move_count = 0; move_count < MoveList.count; move_count++)
    {
        if (str == moveToString(MoveList.moves[move_count]))
            return MoveList.moves[move_count];
    }


    // return empty move, if there was no move matching
    return 0;
}



// UCI::position() is called when engine receives the "position" UCI command.
// The function sets up the position described in the given FEN string ("fen")
// or the starting position ("startpos") and then makes the moves given in the
// following move list ("moves").
void UCI::position(string cmd)
{
    // string variables to parse the command
    istringstream is(cmd);
    string token, fen;


    // move variable to parse the move list
    int m;


    // get the sub-command (startpos or fen)
    is >> token;


    // "startpos" sets the board to the initial position
    if (token == "startpos")
    {
        fen = FENPOS_STARTPOS;
        is >> token; // Consume "moves" token if any
    }


    // "fen" sets the board to a given FEN position
    else if (token == "fen")
    {
        while (is >> token && token != "moves")
            fen += token + " ";
    }


    // if none of "startpos" nor "fen", then return without doing anything
    else
    {
        return;
    }


    // set up the position
    setPosition(fen);


    // parse move list, if any
    while ((is >> token) && ((m = UCI::parseMove(token)) != 0))
    {
        saveBoard();

        // test whether the move is legal and make it on the board
        if (!makeMove(m, AllMoves))
        {
            // undo move, if not legal
            takeBack();
        }
    }
}



// UCI::go() is called when the engine receives the "go" UCI command. The
// function sets the thinking time and other parameters from the input string,
// then starts the search.
void UCI::go(string cmd)
{
    // string variables to parse the command
    istringstream is(cmd);
    string token;


    // reset search configuration before making a new search
    resetLimits();


    // parse sub-commands
    while (is >> token)
             if (token == "wtime")     is >> Limits.wtime;
        else if (token == "btime")     is >> Limits.btime;
        else if (token == "winc")      is >> Limits.winc;
        else if (token == "binc")      is >> Limits.binc;
        else if (token == "movestogo") is >> Limits.movestogo;
        else if (token == "depth")     is >> Limits.depth;
        else if (token == "nodes")     is >> Limits.nodes;
        else if (token == "movetime")  is >> Limits.movetime;
        else if (token == "mate")      is >> Limits.mate;
        else if (token == "perft")     is >> Limits.perft;
        else if (token == "infinite")  Limits.infinite = true;


    // start the search
    search();
}
