#!/bin/bash

# 1. Define variables
EXECUTABLE="./main"           # Name of the executable file
TEST_DIR="./test"             # Directory containing test cases
BUILD_DIR="./build"           # Build directory
OUT_DIR="./out"               # Directory to store output files

# 2. Check if the executable already exists
if [ -f "$EXECUTABLE" ]; then
    echo "Executable $EXECUTABLE found in the root directory. Skipping build steps."
else
    # Create and navigate to the build directory
    echo "Setting up build directory..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Run CMake configuration
    echo "Running CMake..."
    cmake ..
    if [ $? -ne 0 ]; then
        echo "CMake configuration failed!"
        exit 1
    fi

    # Build the project
    echo "Building the project..."
    cmake --build .
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi

    cd ..
fi

# 3. Check if the output directory exists
if [ ! -d "$OUT_DIR" ]; then
    echo "Creating output directory $OUT_DIR..."
    mkdir -p "$OUT_DIR"
fi

# 4. Run test cases and save outputs
echo "Running test cases..."
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable $EXECUTABLE not found!"
    exit 1
fi

if [ ! -d "$TEST_DIR" ]; then
    echo "Error: Test directory $TEST_DIR not found!"
    exit 1
fi

for TEST_FILE in "$TEST_DIR"/*.txt; do
    if [ -f "$TEST_FILE" ]; then
        # Extract the base name of the test file
        BASE_NAME=$(basename "$TEST_FILE" .txt)

        # Define the output file name
        OUTPUT_FILE="$OUT_DIR/output_${BASE_NAME}.txt"

        echo "Running test with input file: $TEST_FILE"
        $EXECUTABLE "$TEST_FILE" > "$OUTPUT_FILE"

        echo "Output saved to $OUTPUT_FILE"
    else
        echo "No .txt files found in $TEST_DIR"
    fi
done

echo "All tests completed!"