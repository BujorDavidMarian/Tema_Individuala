#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <execution>
#include <chrono>
#include <string>
#include <future>
#include <thread>

// Utility to read input
bool readInput(const std::string& filename, std::vector<int>& data) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    int n;
    if (!(file >> n)) return false;
    data.resize(n);
    for (int i = 0; i < n; ++i) {
        if (!(file >> data[i])) break;
    }
    return true;
}

// Utility to verify if sorted
bool verifySorted(const std::vector<int>& data) {
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] < data[i - 1]) return false;
    }
    return true;
}

// --- STL Parallel Merge Sort ---
void parallelMergeSortSTL(std::vector<int>::iterator begin, std::vector<int>::iterator end, int depthLimit) {
    auto size = std::distance(begin, end);
    if (size < 2) return;

    // Base case: use STL parallel sort directly on small chunks
    if (depthLimit <= 0 || size < 2000) {
        std::sort(std::execution::par, begin, end);
        return;
    }

    auto mid = begin + size / 2;

    // Spawn async task for the left half
    auto left_future = std::async(std::launch::async, parallelMergeSortSTL, begin, mid, depthLimit - 1);
    
    // Sort right half on current thread
    parallelMergeSortSTL(mid, end, depthLimit - 1);
    
    // Wait for left half
    left_future.get();

    // Merge the two halves using STL parallel inplace_merge
    std::inplace_merge(std::execution::par, begin, mid, end);
}

// --- STL Parallel Quick Sort ---
void parallelQuickSortSTL(std::vector<int>::iterator begin, std::vector<int>::iterator end, int depthLimit) {
    auto size = std::distance(begin, end);
    if (size < 2) return;

    // Base case: use STL parallel sort on small chunks
    if (depthLimit <= 0 || size < 2000) {
        std::sort(std::execution::par, begin, end);
        return;
    }

    // Pick pivot
    int pivot = *(begin + size / 2);

    // Partition the array using STL parallel partition
    auto mid1 = std::partition(std::execution::par, begin, end, [pivot](int val) { return val < pivot; });
    auto mid2 = std::partition(std::execution::par, mid1, end, [pivot](int val) { return val == pivot; });

    // Spawn async task for the left partition
    auto left_future = std::async(std::launch::async, parallelQuickSortSTL, begin, mid1, depthLimit - 1);
    
    // Sort right partition on current thread
    parallelQuickSortSTL(mid2, end, depthLimit - 1);
    
    // Wait for left partition
    left_future.get();
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <mode: merge|quick> <input_file>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string filename = argv[2];

    std::vector<int> data;
    if (!readInput(filename, data)) {
        std::cerr << "Failed to read input file: " << filename << std::endl;
        return 1;
    }

    int n = (int)data.size();
    std::cout << "Sorting " << n << " elements using C++17 STL " << mode << " sort..." << std::endl;

    // Calculate a reasonable depth limit based on hardware concurrency
    int maxThreads = std::thread::hardware_concurrency();
    int depthLimit = 0;
    while ((1 << depthLimit) < maxThreads) depthLimit++;

    auto start = std::chrono::high_resolution_clock::now();

    if (mode == "merge") {
        parallelMergeSortSTL(data.begin(), data.end(), depthLimit + 2);
    } else if (mode == "quick") {
        parallelQuickSortSTL(data.begin(), data.end(), depthLimit + 2);
    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Sorting finished in " << diff.count() << " seconds." << std::endl;

    if (verifySorted(data)) {
        std::cout << "Verification SUCCESS: Array is sorted." << std::endl;
    } else {
        std::cout << "Verification FAILED: Array is NOT sorted." << std::endl;
    }

    return 0;
}
