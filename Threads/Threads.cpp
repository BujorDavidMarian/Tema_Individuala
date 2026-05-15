#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>

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

// --- Parallel Merge Sort ---

void mergeSortWorker(std::vector<int>::iterator begin, std::vector<int>::iterator end) {
    std::sort(begin, end);
}

void parallelMergeSort(std::vector<int>& data, int numThreads) {
    if (numThreads <= 1 || data.size() < 2) {
        std::sort(data.begin(), data.end());
        return;
    }

    size_t n = data.size();
    size_t chunkSize = n / numThreads;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        auto begin = data.begin() + i * chunkSize;
        auto end = (i == numThreads - 1) ? data.end() : begin + chunkSize;
        threads.emplace_back(mergeSortWorker, begin, end);
    }

    for (auto& t : threads) {
        t.join();
    }

    // Tree-based merge
    int activeChunks = numThreads;
    while (activeChunks > 1) {
        int nextActiveChunks = (activeChunks + 1) / 2;
        for (int i = 0; i < activeChunks / 2; ++i) {
            size_t left = i * 2 * chunkSize;
            size_t mid = (i * 2 + 1) * chunkSize;
            size_t right = (i == nextActiveChunks - 1 || i * 2 + 2 == activeChunks) ? n : (i * 2 + 2) * chunkSize;
            
            // Adjust right boundary for the last merge in a level if necessary
            if (i * 2 + 2 == activeChunks) right = n;

            std::inplace_merge(data.begin() + left, data.begin() + mid, data.begin() + right);
        }
        chunkSize *= 2;
        activeChunks = nextActiveChunks;
    }
}

// --- Parallel Quick Sort ---

int partition(std::vector<int>& data, int low, int high) {
    int pivot = data[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (data[j] <= pivot) {
            i++;
            std::swap(data[i], data[j]);
        }
    }
    std::swap(data[i + 1], data[high]);
    return i + 1;
}

void quickSortRecursive(std::vector<int>& data, int low, int high, int depthLimit) {
    if (low < high) {
        int pi = partition(data, low, high);
        if (depthLimit > 0 && (high - low) > 1000) {
            std::thread t(quickSortRecursive, std::ref(data), low, pi - 1, depthLimit - 1);
            quickSortRecursive(data, pi + 1, high, depthLimit - 1);
            t.join();
        } else {
            quickSortRecursive(data, low, pi - 1, 0);
            quickSortRecursive(data, pi + 1, high, 0);
        }
    }
}

void parallelQuickSort(std::vector<int>& data) {
    int maxThreads = std::thread::hardware_concurrency();
    int depthLimit = 0;
    while ((1 << depthLimit) < maxThreads) depthLimit++;
    quickSortRecursive(data, 0, (int)data.size() - 1, depthLimit);
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
    std::cout << "Sorting " << n << " elements using " << mode << " sort..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    if (mode == "merge") {
        int numThreads = std::thread::hardware_concurrency();
        parallelMergeSort(data, numThreads);
    } else if (mode == "quick") {
        parallelQuickSort(data);
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
