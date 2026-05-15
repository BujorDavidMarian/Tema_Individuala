# Parallel Sorting Algorithms (MPI & Threads)

This project implements various parallel sorting algorithms using two different paradigms:
1. **MPI (Message Passing Interface)** for distributed memory parallelism.
2. **C++ Threads (`std::thread`)** for shared memory parallelism.

Both implementations include **Parallel Merge Sort** and **Parallel Quick Sort**.

## Features
- **MPI Implementation**:
  - Uses `MPI_Scatterv` and `MPI_Gatherv` for data distribution.
  - Tree-based merge for Merge Sort.
  - Recursive communicator splitting for Quick Sort.
- **Threads Implementation**:
  - Uses `std::thread` and `std::inplace_merge` for Merge Sort.
  - Uses recursive task spawning with depth limiting for Quick Sort.
- **Verification**: Both programs automatically verify the sorted result.
- **Timing**: Measures execution time for performance analysis.

## Project Structure
- `MPI1/`: Contains the MPI implementation.
  - `MPI1/MPI1.cpp`: Main source for MPI sorting.
  - `MPI1.sln`: Visual Studio solution.
- `Threads/`: Contains the multi-threaded implementation.
  - `Threads.cpp`: Main source for thread-based sorting.
- `input.txt`: Sample input data (10,000 elements).

## Compilation

### Using MSBuild (Visual Studio)
To compile both projects from the root directory:
```powershell
# Compile MPI Project
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" MPI1\MPI1.sln /p:Configuration=Debug /p:Platform=x64

# Compile Threads Project
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" Threads\Threads.vcxproj /p:Configuration=Debug /p:Platform=x64
```

## Running the Algorithms

### MPI Implementation
Requires `mpiexec` (Microsoft MPI or similar).
```powershell
# Parallel Merge Sort (4 processes)
mpiexec -n 4 "MPI1\x64\Debug\MPI1.exe" merge "input.txt"

# Parallel Quick Sort (4 processes)
mpiexec -n 4 "MPI1\x64\Debug\MPI1.exe" quick "input.txt"
```

### Threads Implementation
Directly execute the binary.
```powershell
# Parallel Merge Sort
"Threads\x64\Debug\Threads.exe" merge "input.txt"

# Parallel Quick Sort
"Threads\x64\Debug\Threads.exe" quick "input.txt"
```

## Input File Format
The input file should be formatted as follows:
```text
<count>
<element1> <element2> ... <elementN>
```
Example:
```text
5
10 5 2 8 3
```
