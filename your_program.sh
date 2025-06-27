#!/bin/sh
#
# Use this script to run your program LOCALLY.
#
# Note: Changing this script WILL NOT affect how CodeCrafters runs your program.
#
# Learn more: https://codecrafters.io/program-interface

set -e # Exit early if any commands fail

# Copied from .codecrafters/compile.sh
#
# - Edit this to change how your program compiles locally
# - Edit .codecrafters/compile.sh to change how your program compiles remotely
(
  cd "$(dirname "$0")" # Ensure compile steps are run within the repository directory
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
  cmake --build ./build
)

# Copied from .codecrafters/run.sh
#
# - Edit this to change how your program runs locally
# - Edit .codecrafters/run.sh to change how your program runs remotely

# Handle different build configurations (Debug/Release on Windows, direct on Linux)
SCRIPT_DIR=$(dirname "$0")
if [ -f "$SCRIPT_DIR/build/Debug/interpreter.exe" ]; then
    exec "$SCRIPT_DIR/build/Debug/interpreter.exe" "$@"
elif [ -f "$SCRIPT_DIR/build/Debug/interpreter" ]; then
    exec "$SCRIPT_DIR/build/Debug/interpreter" "$@"
elif [ -f "$SCRIPT_DIR/build/interpreter.exe" ]; then
    exec "$SCRIPT_DIR/build/interpreter.exe" "$@"
elif [ -f "$SCRIPT_DIR/build/interpreter" ]; then
    exec "$SCRIPT_DIR/build/interpreter" "$@"
else
    echo "Error: Could not find interpreter binary in build directory"
    exit 1
fi
