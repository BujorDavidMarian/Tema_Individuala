#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>

// Helper to merge two sorted vectors
std::vector<int> merge(const std::vector<int>& left, const std::vector<int>& right) {
    std::vector<int> result;
    result.reserve(left.size() + right.size());
    size_t i = 0, j = 0;
    while (i < left.size() && j < right.size()) {
        if (left[i] <= right[j]) {
            result.push_back(left[i++]);
        } else {
            result.push_back(right[j++]);
        }
    }
    while (i < left.size()) result.push_back(left[i++]);
    while (j < right.size()) result.push_back(right[j++]);
    return result;
}

void parallelMergeSort(std::vector<int>& local_data, int rank, int size) {
    // 1. Local Sort
    std::sort(local_data.begin(), local_data.end());

    // 2. Tree-based Merge
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                // Receive from neighbor
                int recv_size;
                MPI_Recv(&recv_size, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::vector<int> recv_data(recv_size);
                MPI_Recv(recv_data.data(), recv_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                // Merge
                local_data = merge(local_data, recv_data);
            }
        } else {
            // Send to neighbor
            int send_size = local_data.size();
            int dest = rank - step;
            MPI_Send(&send_size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            MPI_Send(local_data.data(), send_size, MPI_INT, dest, 0, MPI_COMM_WORLD);
            break; // This process is done
        }
        step *= 2;
    }
}

void parallelQuickSort(std::vector<int>& local_data, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    if (size <= 1) {
        std::sort(local_data.begin(), local_data.end());
        return;
    }

    // Sort local data initially to pick a good pivot
    std::sort(local_data.begin(), local_data.end());

    // Pick pivot (median of rank 0)
    int pivot;
    if (rank == 0) {
        if (!local_data.empty()) {
            pivot = local_data[local_data.size() / 2];
        } else {
            pivot = 0; // Fallback
        }
    }
    MPI_Bcast(&pivot, 1, MPI_INT, 0, comm);

    // Partition local data
    auto it = std::upper_bound(local_data.begin(), local_data.end(), pivot);
    std::vector<int> low_part(local_data.begin(), it);
    std::vector<int> high_part(it, local_data.end());

    // Split processes into two halves
    int mid = size / 2;
    std::vector<int> recv_data;
    if (rank < mid) {
        // I am in the lower half, I want low_parts from upper half
        // and I send my high_parts to upper half
        int partner = rank + mid;
        if (partner < size) {
            // Send high_part
            int send_count = high_part.size();
            MPI_Send(&send_count, 1, MPI_INT, partner, 0, comm);
            if (send_count > 0) MPI_Send(high_part.data(), send_count, MPI_INT, partner, 0, comm);

            // Receive low_part
            int recv_count;
            MPI_Recv(&recv_count, 1, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
            if (recv_count > 0) {
                recv_data.resize(recv_count);
                MPI_Recv(recv_data.data(), recv_count, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
            }
            // Keep my low_part and merge with received low_part
            local_data = merge(low_part, recv_data);
        } else {
            local_data = low_part; // No partner, just keep low
        }
    } else {
        // I am in the upper half
        int partner = rank - mid;
        // Receive high_part
        int recv_count;
        MPI_Recv(&recv_count, 1, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
        if (recv_count > 0) {
            recv_data.resize(recv_count);
            MPI_Recv(recv_data.data(), recv_count, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
        }

        // Send low_part
        int send_count = low_part.size();
        MPI_Send(&send_count, 1, MPI_INT, partner, 0, comm);
        if (send_count > 0) MPI_Send(low_part.data(), send_count, MPI_INT, partner, 0, comm);

        // Keep my high_part and merge with received high_part
        local_data = merge(high_part, recv_data);
    }

    // Split communicator and recurse
    MPI_Comm next_comm;
    MPI_Comm_split(comm, (rank < mid ? 0 : 1), rank, &next_comm);
    parallelQuickSort(local_data, next_comm);
    MPI_Comm_free(&next_comm);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3) {
        if (rank == 0) std::cerr << "Usage: " << argv[0] << " <merge|quick> <input_file>" << std::endl;
        MPI_Finalize();
        return 1;
    }

    std::string algo = argv[1];
    std::string filename = argv[2];

    std::vector<int> data;
    int total_elements = 0;

    if (rank == 0) {
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        infile >> total_elements;
        data.resize(total_elements);
        for (int i = 0; i < total_elements; ++i) {
            infile >> data[i];
        }
        infile.close();
        std::cout << "Read " << total_elements << " elements from " << filename << std::endl;
    }

    // Broadcast total size
    MPI_Bcast(&total_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Start timer
    double start_time = MPI_Wtime();

    // Distribute data
    int chunk_size = total_elements / size;
    int remainder = total_elements % size;

    std::vector<int> send_counts(size);
    std::vector<int> displs(size);
    int offset = 0;
    for (int i = 0; i < size; ++i) {
        send_counts[i] = chunk_size + (i < remainder ? 1 : 0);
        displs[i] = offset;
        offset += send_counts[i];
    }

    std::vector<int> local_data(send_counts[rank]);
    MPI_Scatterv(data.data(), send_counts.data(), displs.data(), MPI_INT,
                 local_data.data(), send_counts[rank], MPI_INT, 0, MPI_COMM_WORLD);

    if (algo == "merge") {
        parallelMergeSort(local_data, rank, size);
        
        // Final gather on Rank 0
        if (rank == 0) {
            // In parallelMergeSort, Rank 0 ends up with the full merged data
            data = local_data;
        }
    } else if (algo == "quick") {
        parallelQuickSort(local_data, MPI_COMM_WORLD);

        // For QuickSort, the data is globally ordered but distributed.
        // We need to gather it back.
        int local_size = local_data.size();
        std::vector<int> all_local_sizes(size);
        MPI_Gather(&local_size, 1, MPI_INT, all_local_sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

        std::vector<int> all_displs(size);
        if (rank == 0) {
            int current_offset = 0;
            for (int i = 0; i < size; ++i) {
                all_displs[i] = current_offset;
                current_offset += all_local_sizes[i];
            }
            data.resize(total_elements);
        }

        MPI_Gatherv(local_data.data(), local_size, MPI_INT,
                    data.data(), all_local_sizes.data(), all_displs.data(), MPI_INT,
                    0, MPI_COMM_WORLD);
    } else {
        if (rank == 0) std::cerr << "Unknown algorithm: " << algo << std::endl;
        MPI_Finalize();
        return 1;
    }

    double end_time = MPI_Wtime();

    if (rank == 0) {
        std::cout << "Sorting finished in " << (end_time - start_time) << " seconds using " << algo << " sort." << std::endl;
        
        // Verify sorting
        bool sorted = true;
        for (size_t i = 1; i < data.size(); ++i) {
            if (data[i] < data[i-1]) {
                sorted = false;
                break;
            }
        }
        if (sorted) {
            std::cout << "Verification SUCCESS: Array is sorted." << std::endl;
        } else {
            std::cout << "Verification FAILED: Array is NOT sorted." << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}