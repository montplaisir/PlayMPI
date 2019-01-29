#include <mpi.h>
#include <iostream>


// Mock evaluator.
// Input: x.
// Output: f.
// Returns: true if eval went OK, false otherwise.
bool eval_x(const double x, double &f)
{
    bool eval_ok = false;

    f = static_cast<int> (x);
    eval_ok = true;

    return eval_ok;
}


int main(int argc, char** argv)
{
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

    int nbPoints = 100;
    if (argc >= 2)
    {
        nbPoints = std::atoi(argv[1]);
    }
    if (0 == world_rank)
    {
        std::cout << "Number of points to evaluate: " << nbPoints << std::endl;
    }

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    if (0 == world_rank)
    {
        for (int pointIndex = 0; pointIndex < nbPoints; pointIndex++)
        {
            int worker_rank = pointIndex % (world_size-1) + 1;
            //std::cout << "pointIndex = " << pointIndex << " worker_rank = " << worker_rank << std::endl;
            // Generate random x.
            // x is between 1 and 100, with 2 decimals.
            double x = (rand() % 10000 + 1) / 100.0;

            // Send X to be evaluated

            // MPI_Send(address, count, datatype, destination, tag, comm)
            // address: Address of the x to evaluate.
            // count: Number of entries starting at address - Here, 1.
            // datatype: Here, MPI_DOUBLE.
            // destination: Rank of the MPI "worker". Here, given by the for loop.
            // tag: Used for message matching - Here, 0 for master-to-worker, 1 for worker-to-master.
            // comm: Communication context - Here, MPI_COMM_WORLD.
            std::cout << "Master sends " << x << " to worker " << worker_rank << std::endl;
            MPI_Send(&x, 1, MPI_DOUBLE, worker_rank, 0, MPI_COMM_WORLD);
        }

        for (int pointIndex = 0; pointIndex < nbPoints; pointIndex++)
        {
            int worker_rank = pointIndex % (world_size-1) + 1;
            // Receive evaluations
            double evalpoint[3];
            MPI_Status status;
            MPI_Recv(&evalpoint, 3, MPI_DOUBLE, worker_rank, 1, MPI_COMM_WORLD, &status);
            int nbReceived = 0;
            MPI_Get_count(&status, MPI_DOUBLE, &nbReceived);
            if (nbReceived < 3)
            {
                std::cerr << "Error: Worker expected 3 values" << std::endl;
            }
            double x = evalpoint[0];
            double f = evalpoint[1];
            double eval_ok = evalpoint[2];
            std::cout << "Received from worker " << worker_rank << ": X = " << x << " f = " << f << " eval_ok = " << eval_ok << std::endl;
        }
    }
    else
    {
        // Receive X
        // MPI_Recv(address, maxcount, datatype, source, tag, comm, status)
        // address: Address of the x to evaluate.
        // maxcount: Number of entries starting at address - Here, 1.
        // datatype: Here, MPI_DOUBLE.
        // source: Rank of the MPI "master". Here, always 0.
        // tag: Used for message matching - Here, 0 for master-to-worker, 1 for worker-to-master.
        // comm: Communication context - Here, MPI_COMM_WORLD.
        // status: Information about the actual message size, source, and tag.
        //
        double x = 0.0;
        MPI_Status status;
        MPI_Recv(&x, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        int nbReceived = 0;
        MPI_Get_count(&status, MPI_DOUBLE, &nbReceived);
        if (nbReceived < 1)
        {
            std::cerr << "Error: Worker expected 1 value" << std::endl;
        }
        std::cout << "Worker " << world_rank << " on " << processor_name << " Received x = " << x << std::endl;
        // Evaluate X
        double f = 0.0;
        bool eval_ok = eval_x(x, f);

        // Send back evaluation to master
        // Although only f is new info, sending both x and f in the same message
        // seems easier to manage.
        // Also include eval_ok.
        // Sending as 3 double for simplicity.
        double evalpoint[3];
        evalpoint[0] = x;
        evalpoint[1] = f;
        evalpoint[2] = eval_ok;

        MPI_Send(&evalpoint, 3, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
        std::cout << "Worker " << world_rank << " send " << evalpoint[0] << " " << evalpoint[1] << " " << evalpoint[2] << std::endl;
    }


    // Finalize the MPI environment.
    MPI_Finalize();
}

