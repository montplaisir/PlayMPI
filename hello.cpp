#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {

    // Usage: mpirun -np <number of processes> -f <hostfile> hello

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_size <= 1 && 0 == world_rank)
    {
        std::cout << "Usage: mpirun -np <number of processes> -f <hostfile> " << argv[0] << std::endl;
        return 1;
    }

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    std::cout << "Hello from processor " << processor_name;
    std::cout << ", rank " << world_rank << " out of " << world_size << " processors" << std::endl;

    // Finalize the MPI environment.
    MPI_Finalize();
}

