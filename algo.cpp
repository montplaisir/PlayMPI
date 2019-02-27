#include <mpi.h>
#include <iostream>
#include <vector>

#include "EvaluatorControl.hpp"

int main(int argc, char** argv)
{
    // Usage: mpirun -np <number of processes> -f <hostfile> algo

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // initialize random seed
    srand(time(NULL));

    // Get the number of processes
    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Get the rank of the process
    int worldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

    if (worldSize <= 1 && 0 == worldRank)
    {
        std::cout << "Usage: mpirun -np <nb of processes> -f <hostfile> " << argv[0] << " [nb of points to eval]" << std::endl;
        return 1;
    }

    std::cout << "VRM: Launch " << argv[0] << " rank " << worldRank << std::endl;
    // Start EvaluatorControl 
    EvaluatorControl evc;
    evc.run();

    // The rest of the algo is in master only
    if (0 == worldRank)
    {
        std::cout << "VRM: Generate points in rank " << worldRank << "... etc." << std::endl;
        std::vector<double> pointsVector = generatePoints(nbPoints);
    }
    /*
    int nbPoints = 100;
    if (argc >= 2)
    {
        nbPoints = std::atoi(argv[1]);
    }
    if (0 == worldRank)
    {
        std::cout << "Number of points to evaluate: " << nbPoints << std::endl;
    }

    // Master generates and sends nbPoints to workers for evaluation.
    if (0 == worldRank)
    {
        std::vector<double> pointsVector = generatePoints(nbPoints);
        std::vector<EvalPoint> evalpointVector;
        sendPointsToWorkers(pointsVector, worldSize);
        bool allPointsReceived = false;
        while (!allPointsReceived)
        {
            allPointsReceived = receiveEvaluatedPoints(worldSize, nbPoints, evalpointVector);
        }
        // All points received, master is done.

        // Send word to workers that evaluations are done, so that they stop "listening".
        sendEvaluationDoneToWorkers(worldSize);
        // Wait for all workers to have acknowledged they are done.
        waitAllWorkersDone(worldSize);

        // Print all points.
        std::cout << std::endl << "Summary of " << evalpointVector.size() << " evalpoints:" << std::endl;
        std::cout << "X\tF\tProcess" << std::endl;
        for (int i = 0; i < evalpointVector.size(); i++)
        {
            EvalPoint ep = evalpointVector[i];
            std::cout << ep.getX() << "\t" << ep.getF() << "\t" << ep.getWorker() << std::endl;
        }

    }

    */

    // Finalize the MPI environment.
    MPI_Finalize();
}

