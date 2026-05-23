#!/bin/bash

# Configuration
VERSION="2.1"
DIST_DIR="../dist"
mkdir -p "$DIST_DIR"

# Compilers
CXX_MAC="g++"
CXX_WIN_X64="x86_64-w64-mingw32-g++"
CXX_LINUX_X64="/opt/homebrew/Cellar/x86_64-unknown-linux-gnu/13.3.0/bin/x86_64-unknown-linux-gnu-g++"
CXX_LINUX_ARM64="/opt/homebrew/bin/aarch64-unknown-linux-gnu-g++"

# Helper function
build_target() {
    ARCH=$1
    CXX=$2
    OUT_NAME=$3
    
    echo "--------------------------------------------------"
    echo "Building for $ARCH..."
    echo "Compiler: $CXX"
    
    # Check if compiler exists
    if ! command -v "$CXX" &> /dev/null && [ ! -f "$CXX" ]; then
        echo "Error: Compiler $CXX not found. Skipping."
        return
    fi
    
    make clean > /dev/null
    make -j8 ARCH=$ARCH CXX=$CXX
    
    if [ $? -eq 0 ]; then
        if [[ "$ARCH" == *"win"* ]]; then
            mv gargantua.exe "$DIST_DIR/$OUT_NAME.exe"
        else
            mv gargantua "$DIST_DIR/$OUT_NAME"
        fi
        echo "Success: $OUT_NAME created."
    else
        echo "Failed to build $ARCH"
    fi
}

# 1. macOS (Apple Silicon)
# Note: Usually just 'make' defaults to native, but we can be explicit
build_target "apple-silicon" "$CXX_MAC" "gargantua-$VERSION-macos-arm64"

# 2. Windows x64
build_target "win-x64" "$CXX_WIN_X64" "gargantua-$VERSION-windows-x64"

# 3. Linux x64
build_target "x86-64" "$CXX_LINUX_X64" "gargantua-$VERSION-linux-x64"

# 4. Linux ARM64
build_target "armv8" "$CXX_LINUX_ARM64" "gargantua-$VERSION-linux-arm64"

# 5. Windows ARM64 (Skipped - Compiler Missing)
echo "--------------------------------------------------"
echo "Skipping Windows ARM64 (Toolchain not present)"
echo "To build manually: make ARCH=win-arm64 CXX=aarch64-w64-mingw32-g++"

echo "--------------------------------------------------"
echo "Builds complete. Check $DIST_DIR"
ls -lh "$DIST_DIR"
