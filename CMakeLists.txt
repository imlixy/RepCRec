# Set the minimum required CMake version and project name
cmake_minimum_required(VERSION 3.10)
project(RepCRec)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the source file directory
# Specify source and include directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# Search for all .cpp files
file(GLOB_RECURSE SRCS ${SRC_DIR}/*.cpp)

# Add an executable target
add_executable(main ${SRCS})

# Include the header files from the /include directory
target_include_directories(main PRIVATE ${INCLUDE_DIR})

# Set the output directory
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Platform-specific compiler options
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Default to Debug build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
