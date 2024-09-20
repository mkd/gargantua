# This file is part of Gargantua, a UCI chess engine with NNUE evaluation
# derived from Chess0, and inspired by Code Monkey King's bbc-1.4.
#     
# Copyright (C) 2024 Claudio M. Camacho
#
# Gargantua is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Gargantua is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.    If not, see <http://www.gnu.org/licenses/>.


### Executable name
APP = gargantua
EXE = gargantua.exe
DBG = gargantua.dbg


### Compiler
CC  := g++
WC  := x86_64-w64-mingw32-g++


### Detect architecture
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

### Preprocessor defines
DEFINES = -DIS_64BIT

### Architecture-specific flags and defines
ARCH_FLAGS =
ARCH_DEFINES =

ifeq ($(UNAME_M),x86_64)
  # x86_64-specific compiler flags and defines
  ARCH_FLAGS += -mavx2 -mbmi -mlzcnt -mbmi2 -msse4.1 -msse3 -msse2 -msse -mpopcnt
  ARCH_DEFINES += -DUSE_AVX2 -DUSE_BMI1 -DUSE_BMI2 -DUSE_SSE41 -DUSE_SSE3 -DUSE_SSE2 -DUSE_SSE -DUSE_POPCNT
else ifeq ($(UNAME_M),arm64)
  # ARM64-specific compiler flags and defines
  ARCH_FLAGS += -mcpu=apple-m1
  ARCH_DEFINES += -DUSE_ARM_NEON
endif


### Source and objects files
SOURCES   := $(wildcard *.cpp)
OBJECTS   := $(SOURCES:.cpp=.o)
DOBJECTS  := $(SOURCES:.cpp=.dbo)
WOBJECTS  := $(SOURCES:.cpp=.obj)
DEPFILES  := $(SOURCES:.cpp=.d)


### Compilation flags
CXXFLAGS := -Ofast -Wall -Wcast-qual -pedantic -std=c++20 -Wvla -fno-exceptions -fno-rtti -flto -pthread -DNDEBUG $(DEFINES) $(ARCH_DEFINES) $(ARCH_FLAGS)
DFLAGS   := -Wall -O0 -g -DDEBUG -pthread -std=c++20 $(DEFINES) $(ARCH_DEFINES) $(ARCH_FLAGS)
WINFLAGS := -DWIN64 -pthread -static -static-libgcc -static-libstdc++ -DNDEBUG $(DEFINES) $(ARCH_DEFINES) $(ARCH_FLAGS)
LDFLAGS  += -lpthread


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
	@echo ' Compiling [win64] $@' && $(WC) $(CXXFLAGS) $(WINFLAGS) -c $< -o $@ -lm

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
	$(CC) -o $@ $(DOBJECTS) $(LDFLAGS)

debug: $(DBG)

$(EXE): $(WOBJECTS)
	@echo ' Linking   [win64] $@'
	$(WC) -o $@ $(WOBJECTS) $(WINFLAGS) $(LDFLAGS)

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

