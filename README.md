# Parallel Sorting Algorithms (MPI, Threads, & STL)

This project implements various parallel sorting algorithms using three different paradigms:
1. **MPI (Message Passing Interface)** for distributed memory parallelism.
2. **C++ Threads (`std::thread`)** for shared memory parallelism.
3. **C++17 STL Parallel Algorithms** for high-level shared memory parallelism.

Both MPI and Threads implementations include **Parallel Merge Sort** and **Parallel Quick Sort**. The STL implementation uses recursive tasks with `std::async` and parallel execution policies.

## Performance Comparison
Below are the benchmark results comparing the average execution time of the three implementations (MPI with 4 processes, C++ Threads, and C++17 STL) for a dataset of 10,000 elements.

### Merge Sort Comparison
<img width="3000" height="1800" alt="Merge_Sort_Comparison" src="https://github.com/user-attachments/assets/8cd09e01-4dbf-470e-be4e-fa0061c8037a" />

### Quick Sort Comparison
<img width="3000" height="1800" alt="Quick_Sort_Comparison" src="https://github.com/user-attachments/assets/050595b2-27fe-4047-ab53-df26f5aafc64" />


## Features
- **MPI Implementation**:
  - Uses `MPI_Scatterv` and `MPI_Gatherv` for data distribution.
  - Tree-based merge for Merge Sort.
  - Recursive communicator splitting for Quick Sort.
- **Threads Implementation**:
  - Uses `std::thread` and `std::inplace_merge` for Merge Sort.
  - Uses recursive task spawning with depth limiting for Quick Sort.
- **STL Implementation**:
  - Uses C++17 `std::execution::par` for parallel sorting.
- **Verification**: All programs automatically verify the sorted result.
- **Timing**: Measures execution time for performance analysis.

## Project Structure
- `MPI1/`: Contains the MPI implementation.
  - `MPI1/MPI1.cpp`: Main source for MPI sorting.
  - `MPI1.sln`: Visual Studio solution.
- `Threads/`: Contains the multi-threaded implementation.
  - `Threads.cpp`: Main source for thread-based sorting.
- `STL/`: Contains the pure STL parallel implementation.
  - `STL.cpp`: Main source for STL parallel sorting.
- `input.txt`: Sample input data (10,000 elements).

## Compilation

### Using MSBuild (Visual Studio)
To compile the projects from the root directory:
```powershell
# Compile MPI Project
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" MPI1\MPI1.sln /p:Configuration=Debug /p:Platform=x64

# Compile Threads Project
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" Threads\Threads.vcxproj /p:Configuration=Debug /p:Platform=x64

# Compile STL Project
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" STL\STL.vcxproj /p:Configuration=Debug /p:Platform=x64
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
& "Threads\x64\Debug\Threads.exe" merge "input.txt"

# Parallel Quick Sort
& "Threads\x64\Debug\Threads.exe" quick "input.txt"
```

### STL Implementation
Directly execute the binary.
```powershell
# STL Parallel Sort
& "STL\x64\Debug\STL.exe" "input.txt"
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
