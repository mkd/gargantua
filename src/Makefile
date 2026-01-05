# Gargantua Makefile
# Copyright (C) 2026 Claudio M. Camacho

# --- 1. General Settings ---
APP      = gargantua
EXE      = gargantua.exe

# Compiler and Flags
CXX      := g++
# -O3: Max optimization
# -flto: Link Time Optimization (cross-file optimization)
# -funroll-loops: Unroll loops for speed (at cost of binary size)
CXXFLAGS := -std=c++20 -O3 -flto -funroll-loops -DNDEBUG -pedantic -Wvla \
            -fno-exceptions -fno-rtti -pthread -DUSE_NEW_NNUE
LDFLAGS  := -lpthread -flto

# OS and Arch Detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Default ARCH if not specified
ARCH ?= auto

# --- 2. Architecture Auto-Detection ---
ifeq ($(ARCH),auto)
    ifeq ($(UNAME_S),Darwin)
        ifeq ($(UNAME_M),arm64)
            ARCH = apple-silicon
        else
            ARCH = x86-64
        endif
    else ifeq ($(UNAME_S),Linux)
        ifeq ($(UNAME_M),aarch64)
            ARCH = armv8
        else
            ARCH = x86-64
        endif
    else ifneq (,$(findstring MINGW,$(UNAME_S)))
        ARCH = win-x64
    else ifneq (,$(findstring MSYS,$(UNAME_S)))
        ARCH = win-x64
    endif
endif

# --- 3. Architecture Specific Flags ---
# Note on Cross-Compilation:
# You can cross-compile for any target (e.g., Linux -> Windows, macOS -> Linux)
# *provided* you have the correct toolchain installed and accessible in your PATH.
# For example, to build for Linux x86_64 from macOS, you need 'x86_64-linux-gnu-g++'.
# You can override the compiler with: make ARCH=x86-64 CXX=x86_64-linux-gnu-g++

# Apple Silicon (macOS)
ifeq ($(ARCH),apple-silicon)
    # Apple M1/M2/M3 supports NEON and DotProduct
    CXXFLAGS += -mcpu=apple-m1 -DUSE_NEON -DUSE_NEON_DOTPROD -DUSE_POPCNT
    LDFLAGS  += 

# Linux / Generic x86-64 (Modern)
else ifeq ($(ARCH),x86-64)
    # Target Haswell+ (AVX2). Enables AVX2, BMI2, POPCNT, PEXT.
    CXXFLAGS += -mavx2 -mbmi -mbmi2 -mlzcnt -mpopcnt -msse4.1 \
                -DUSE_AVX2 -DUSE_BMI2 -DUSE_SSE41 -DUSE_SSSE3 -DUSE_SSE2 \
                -DUSE_POPCNT -DUSE_PEXT

# Linux ARM64
else ifeq ($(ARCH),armv8)
    CXXFLAGS += -march=armv8-a -DUSE_NEON -DUSE_POPCNT

# Windows x64
else ifeq ($(ARCH),win-x64)
    CXX      := x86_64-w64-mingw32-g++
    CXXFLAGS += -mavx2 -mbmi -mbmi2 -mlzcnt -mpopcnt -msse4.1 -DWIN64 \
                -DUSE_AVX2 -DUSE_BMI2 -DUSE_SSE41 -DUSE_SSSE3 -DUSE_SSE2 \
                -DUSE_POPCNT -DUSE_PEXT
    LDFLAGS  += -static -static-libgcc -static-libstdc++ 
    APP      := $(EXE)

# Windows ARM64 (Experimental/Future)
else ifeq ($(ARCH),win-arm64)
    # Assumes a mingw-w64 toolchain for aarch64 exists on the system
    CXX      := aarch64-w64-mingw32-g++
    CXXFLAGS += -march=armv8-a -DWIN64
    LDFLAGS  += -static -static-libgcc -static-libstdc++
    APP      := $(EXE)

endif


# --- 4. Sources and Objects ---
SOURCES_ROOT  := $(wildcard *.cpp)
PROBE_SOURCES := $(wildcard stockfish_probe/*.cpp) \
                 stockfish_probe/nnue/evaluate_nnue.cpp \
                 stockfish_probe/nnue/features/half_ka_v2_hm.cpp

SOURCES := $(SOURCES_ROOT) $(PROBE_SOURCES)
OBJECTS := $(SOURCES:.cpp=.o)
DEPS    := $(SOURCES:.cpp=.d)

# --- 5. Build Rules ---

.PHONY: all clean help

all: $(APP)
	@echo "Build complete for ARCH=$(ARCH). Binary: $(APP)"

$(APP): $(OBJECTS)
	@echo "Linking $@"
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependencies
-include $(DEPS)

clean:
	@echo "Cleaning..."
	rm -f $(OBJECTS) $(DEPS) $(APP) $(EXE) gargantua.dbg

help:
	@echo "Gargantua Makefile Help"
	@echo ""
	@echo "Usage:"
	@echo "  make                 Builds for the detected host architecture: $(ARCH)"
	@echo "  make ARCH=...        Builds for a specific architecture."
	@echo ""
	@echo "Supported Architectures:"
	@echo "  apple-silicon        macOS on Apple Silicon (M1/M2/M3)"
	@echo "  x86-64               Generic 64-bit x86 (AVX2)"
	@echo "  armv8                Generic 64-bit ARM (Linux)"
	@echo "  win-x64              Windows 64-bit (Cross-compile via MinGW)"
	@echo "  win-arm64            Windows ARM64 (Cross-compile via MinGW)"
	@echo ""
