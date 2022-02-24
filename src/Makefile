# This file is part of Gargantua, a UCI chess engine derived from Chess0 which
# uses NNUE for evaluation. This engine has been inspired by BBC-1.4 (by Code
# Monkey King), Blunder (by Christian Dean) and Stockfish.
#	 
# Copyright (C) 2022 Claudio M. Camacho
#
# Gargantua is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Gargantua is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.	If not, see <http://www.gnu.org/licenses/>.



### Executable name
APP = gargantua
EXE = gargantua.exe
DBG = gargantua.dbg



### Compiler
CC	:= g++
WC	:= x86_64-w64-mingw32-g++



############################
# SIMD flags (for NNUE)
#--------------
# Disable/Add flags as needed
############################
DEFINES =
DEFINES += -DIS_64BIT
ifeq ($(COMP),$(filter $(COMP),gcc clang win icpc))
  DEFINES += -DUSE_AVX2 -mavx2
  DEFINES += -DUSE_SSE41 -msse4.1
  DEFINES += -DUSE_SSE3 -msse3
  DEFINES += -DUSE_SSE2 -msse2
  DEFINES += -DUSE_SSE -msse
endif



### Source and objects files
SOURCES   := $(wildcard *.cpp)
OBJECTS   := $(SOURCES:.cpp=.o)
DOBJECTS  := $(SOURCES:.cpp=.dbo)
WOBJECTS  := $(SOURCES:.cpp=.obj)
DEPFILES  := $(SOURCES:.cpp=.d)



### Compilation flags
CXXFLAGS := -Ofast -Wall -Wcast-qual -std=c++17 -fno-exceptions -fno-rtti -m64 -mpopcnt -flto $(DEFINES)
DFLAGS   := -Wall -O0 -g -DDEBUG -std=c++17 $(DEFINES)



### Suffixes
.SUFFIXES:
.SUFFIXES: .d .cpp .o .dbo .obj

%.d: %.cpp
	@echo ' Generating deps   $@' && $(CC) -MM $< | \
	sed '1 s,^[^:]*,$@ $(@:.d=.o) $(@:.d=.dbo) $(@:.d=.obj),' > $@

%.o: %.cpp
	@echo ' Compiling         $@' && $(CC) $(CXXFLAGS) -c $< -o $@

%.dbo: %.cpp
	@echo ' Compiling [debug] $@' && $(CC) $(DFLAGS) -c $< -o $@

%.obj: %.cpp
	@echo ' Compiling [win64] $@' && $(WC) $(CXXFLAGS) -c $< -o $@ -lm

%:: ;



### Phony declarations
.PHONY: clean help



### Build targets
all: $(APP)


clean:
	@echo -n 'Deleting object files: '
	@rm -fr $(OBJECTS) $(DOBJECTS) $(WOBJECTS) $(DEPFILES)
	@rm -fr gargantua gargantua.exe gargantua.dbg
	@echo 'done.'


$(APP): $(OBJECTS)
	@echo ' Linking           $@'
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)


$(DBG): $(DOBJECTS)
	@echo ' Linking   [debug] $@'
	$(CC) -o $@ $(DOBJECTS)


$(EXE): $(WOBJECTS)
	@echo ' Linking   [win64] $@'
	$(WC) -o $@ $(WOBJECTS) $(LDFLAGS)


exe: $(EXE)


help:
	@echo 'User targets:'; \
	echo ''; \
	echo ' all        - Default target.  Alias for "gargantua".'; \
	echo ' gargantua  - Build the binary.'; \
	echo ' exe        - Build the binary for Win64 architecture.'; \
	echo ' debug      - Build the debug binary.'; \
	echo ' clean      - Remove objects, dependency files and binaries.'; \
	echo ''



# Generate dependencies except for these targets
NODEP_TARGETS := clean help

ifeq ($(filter $(NODEP_TARGETS),$(MAKECMDGOALS)),)
-include $(DEPFILES)
endif
