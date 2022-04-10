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

#ifndef UCI_H
#define UCI_H

#include <map>



// Engine information 
#define ENGINE_NAME     "Gargantua"
#define ENGINE_VERSION  "1.0"
#define ENGINE_AUTHOR   "Claudio M. Camacho <claudiomkd@gmail.com>"
#define ENGINE_URL      "http://github.com/mkd/gargantua"



// Default sizes for the Hash option
#define HASH_MIN_SIZE   16
#define HASH_MAX_SIZE 1024



// UCI interface functionality, including move parsing, UCI commands, etc.
namespace UCI 
{

string moveToString(int m);
int parseMove(string);
void position(istringstream &);
void go(istringstream &);
void setOption(istringstream &);
void traceEval();
void loop(int argc, char *argv[]);
void printHelp();
void resetOptions();

}  //  namespace UCI



#endif  //  UCI_H
