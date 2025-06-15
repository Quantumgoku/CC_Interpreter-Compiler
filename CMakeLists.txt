cmake_minimum_required(VERSION 3.13)

project(codecrafters-interpreter)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

# Ensure LoxFunction.cpp is included in the build
list(APPEND SOURCE_FILES src/LoxFunction.cpp)

# Remove astnodegenerator.cpp from the source files for the main interpreter
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*astnodegenerator\\.cpp$")

add_executable(interpreter ${SOURCE_FILES})

# Add astnodegenerator as a separate executable, but do not build it by default
add_executable(astnodegenerator src/astnodegenerator.cpp)
set_target_properties(astnodegenerator PROPERTIES EXCLUDE_FROM_ALL TRUE)
# To build manually: cmake --build . --target astnodegenerator