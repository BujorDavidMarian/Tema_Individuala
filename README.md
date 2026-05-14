# MPI Parallel Sorting Algorithms

This project implements two parallel sorting algorithms using **MPI (Message Passing Interface)** and **C++ STL**:
1. **Parallel Merge Sort**: Uses a tree-based reduction pattern for merging.
2. **Parallel Quick Sort**: Uses a distributed pivot partitioning approach.

## Features
- **Data Distribution**: Data is distributed using `MPI_Scatterv` and collected using `MPI_Gatherv`.
- **Timing**: Measures execution time using `MPI_Wtime()`.
- **Verification**: Automatically verifies if the resulting array is correctly sorted.
- **Input Handling**: Reads data from a text file (first line is the count, followed by space-separated integers).

## Project Structure
- `MPI1/MPI1/MPI1.cpp`: The main source code containing both algorithm implementations.
- `generate_input.py`: A Python script to generate random test data (`input.txt`).
- `MPI1/MPI1.sln`: Visual Studio solution for the project.

## Compilation
To compile the project using MSBuild (Visual Studio 2022):
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" MPI1\MPI1.sln /p:Configuration=Debug /p:Platform=x64
```

## Running the Algorithms
The program requires two arguments: the algorithm name (`merge` or `quick`) and the path to the input file.

### Parallel Merge Sort
Run with 4 processes:
```powershell
mpiexec -n 4 "MPI1\x64\Debug\MPI1.exe" merge "input.txt"
```

### Parallel Quick Sort
Run with 4 processes:
```powershell
mpiexec -n 4 "MPI1\x64\Debug\MPI1.exe" quick "input.txt"
```

## Input File Format
The input file should be formatted as follows:
```text
10
5 2 9 1 5 6 3 8 7 4
```
Where `10` is the number of elements.
