### Overview

This project implements a simple distributed database system, complete with serializable snapshot isolation, replication, and failure recovery.

For more detailed design content, please refer to the [design document](https://github.com/imlixy/RepCRec/blob/main/design.md).

### Project Structure

```bash
/RepCRec
├── /src                	# Source code files (.cpp)
├── /include            	# Header files (.h)
├── /test               	# Sample input files for testing
├── /out                	# Output directory for test results (optional)
├── CMakeLists.txt      	# CMake configuration file
├── test_cases_overview.txt	# Descriptions and analysis of all test cases under /test
├── build_and_run.sh    	# Script for automating build and test execution
├── RepCRec-package.rpz 	# File used to run when using Reprozip
├── design.md  				# Design document detailing modules and functions
├── README.md           	# simple file guide how to build and run the project
```

## Build

#### Prerequisites

To run this project, you will need the following:

- **CMake** >= 3.10
- **GNU Make** >= 3.8
- A C++ compiler with C++17 support (e.g., **g++** >= 7.0)
- Optional: **Reprozip** (if using packed execution)

#### a. Automatically build and run

A convenient script `build_and_run.sh` is provided to build the project, run all test cases in the `test/` directory, and save their outputs to the `out/` directory

1. Make sure `build_and_run.sh` is executable:

   ```bash
   chmod +x build_and_run.sh
   ```

2. Run the script:

   ```powershell
   ./build_and_run.sh
   ```

#### b. Using `CMake`

1. **Build the project**:

   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

   After successful compilation, you will find the executable file  `main`  under the project root directory.

2. **Run the project**:

   - **File input mode** (file under directory `/test`):

     ```bash
     ./main <input-file>
     ```

   - **Interactive mode** (input from stdin):

     ```bash
     ./main
     ```

#### c. Using `Reprozip`

Reprozip allows you to run this project in a reproducible environment.

1. Ensure Reprozip is installed on your machine or virtual environment. You can install it via pip:

   ```bash
   pip install reprozip
   ```

3. Set up and execute with Reprozip:

   ```bash
   reprounzip directory setup RepCRec-package.rpz ./example
   reprounzip directory run ./example
   ```

3. After you run this, you can navigate into the `example` directory, you will find the `RepCRec` folder within which we will have:

   ```bash
   /example
   ├── build_and_run.sh	# the automation script
   ├── main      			# the executable file
   ├── /test
   ├── /out
   ```

4. Now you can run the project:

   ```bash
   # 1. file input mode (file under directory ./test)
   ./main <input-file>
   
   # 2. interactive mode (input from stdin)
   ./main
   ```

##### Author

- Xinyu Li (xl5280@nyu.edu)