#include <mpi.h>
#include <iostream>


// Mock evaluator.
// Input: x.
// Output: f.
// Returns: true if eval went OK, false otherwise.
bool eval_x(const double x, double f)
{
    bool eval_ok = false;

    f = static_cast<int> (x);
    eval_ok = true;

    return eval_ok;
}


int main(int argc, char** argv) {

    // Usage: mpirun -np <number of processes> -f <hostfile> hello

    // initialize random seed
    srand(time(NULL));

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
    //std::cout << "Hello from processor " << processor_name;
    //std::cout << ", rank " << world_rank << " out of " << world_size << " processors" << std::endl;

    if (0 == world_rank)
    {
        for (int worker_rank = 1; worker_rank < world_size; worker_rank++)
        {
            // Generate random x.
            // x is between 1 and 100, with 2 decimals.
            double x = (rand() % 10000 + 1) / 100.0;
            std::cout << "generated x = " << x << std::endl;

            // Send X to be evaluated

            // MPI_Send(address, count, datatype, destination, tag, comm)
            // address: Address of the x to evaluate.
            // count: Number of entries starting at address - Here, 1.
            // datatype: Here, MPI_DOUBLE.
            // destination: Rank of the MPI "worker". Here, given by the for loop.
            // tag: Used for message matching - Here, 0 (ignored).
            // comm: Communication context - Here, MPI_COMM_WORLD.
            MPI_Send(&x, 1, MPI_DOUBLE, worker_rank, 0, MPI_COMM_WORLD);
        }
        // Receive evaluation
    }
    else
    {
        // Receive X
        // MPI_Recv(address, maxcount, datatype, source, tag, comm, status)
        // address: Address of the x to evaluate.
        // maxcount: Number of entries starting at address - Here, 1.
        // datatype: Here, MPI_DOUBLE.
        // source: Rank of the MPI "master". Here, always 0.
        // tag: Used for message matching - Here, 0 (ignored).
        // comm: Communication context - Here, MPI_COMM_WORLD.
        // status: Information about the actual message size, source, and tag.
        //
        double x = 0.0;
        MPI_Status status;
        MPI_Recv(&x, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        std::cout << "Worker " << world_rank << " on " << processor_name << " Received x = " << x << std::endl;
        // Evaluate X
        int f = 0.0;
        bool eval_ok = eval_x(x, f);
        // Send evaluation
    }


    // Finalize the MPI environment.
    MPI_Finalize();
}

