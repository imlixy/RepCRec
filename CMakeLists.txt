# Set the minimum required CMake version and project name
cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the source file directory
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Search for all .cpp files
file(GLOB_RECURSE SRCS ${SRC_DIR}/*.cpp)

# Add an executable target
add_executable(main ${SRCS})

# Set the output directory
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
