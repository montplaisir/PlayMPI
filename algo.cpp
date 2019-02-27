#include <mpi.h>
#include <iostream>
#include <vector>

#include "EvalPoint.hpp"
#include "EvaluatorControl.hpp"

// Generate nbPoints random values between 1 and 100
std::vector<double> generatePoints(const int nbPoints)
{
    std::vector<double> points;

    for (int i = 0; i < nbPoints; i++)
    {
        // Generate random x.
        // x is between 1 and 100, with 2 decimals.
        double x = (rand() % 10000 + 1) / 100.0;
        points.push_back(x);
    }

    return points;
}


// Send nbPoints to worldSize-1 workers.
void sendPointsToWorkers(const std::vector<double> pointsVector, const int worldSize)
{
    int nbPoints = pointsVector.size();
    for (int pointIndex = 0; pointIndex < nbPoints; pointIndex++)
    {
        int workerRank = pointIndex % (worldSize-1) + 1;
        // MPI_Send(address, count, datatype, destination, tag, comm)
        // address: Address of the x to evaluate.
        // count: Number of entries starting at address - Here, 1.
        // datatype: Here, MPI_DOUBLE.
        // destination: Rank of the MPI "worker". Here, given by the for loop.
        // tag: Used for message matching - Here, 0 for master-to-worker, 1 for worker-to-master.
        // comm: Communication context - Here, MPI_COMM_WORLD.
        double x = pointsVector[pointIndex];
        //std::cout << "Master sends " << x << " to worker " << workerRank << std::endl;
        MPI_Send(&x, 1, MPI_DOUBLE, workerRank, tagPointToEvaluate, MPI_COMM_WORLD);
    }
}


// Master receives evaluated points.
bool receiveEvaluatedPoints(const int worldSize, const int nbPoints, std::vector<EvalPoint> &evalpointVector)
{
    bool allPointsEvaluated = false;

    // Go around all workers.
    for (int workerRank = 1; workerRank < worldSize; workerRank++)
    {
        // Probe if there is a Send from that worker waiting to be received.
        // MPI_Iprobe(source, tag, comm, flag, status)
        MPI_Status status;
        int newEvalPointReceived = 0;
        MPI_Iprobe(workerRank, tagEvaluatedPoint, MPI_COMM_WORLD, &newEvalPointReceived, &status);
        if (newEvalPointReceived > 0)
        {
            // Actually receive evaluation.
            double xfe[3];
            MPI_Status status;
            MPI_Recv(&xfe, 3, MPI_DOUBLE, workerRank, 1, MPI_COMM_WORLD, &status);
            double x = xfe[0];
            double f = xfe[1];
            double eval_ok = xfe[2];
            EvalPoint evalpoint(x, f, eval_ok, workerRank);
            evalpointVector.push_back(evalpoint);
            if (evalpointVector.size() == nbPoints)
            {
                allPointsEvaluated = true;
            }
        }
    }

    return allPointsEvaluated;
}


// Master sends word to workers that evaluations are done.
void sendEvaluationDoneToWorkers(const int worldSize)
{
    int done = 1;
    for (int workerRank = 1; workerRank < worldSize; workerRank++)
    {
        MPI_Send(&done, 1, MPI_INT, workerRank, tagEvaluationDone, MPI_COMM_WORLD);
    }
}


// Master receives "done" from workers, until we get worldSize.
void waitAllWorkersDone(const int worldSize)
{
    bool allWorkersDone = false;

    MPI_Status status;
    int flagDone = 0;
    int workerDone = 0;
    int nbWorkersDone = 0;

    while (!allWorkersDone)
    {
        for (int workerRank = 1; workerRank < worldSize && !allWorkersDone; workerRank++)
        {
            MPI_Iprobe(workerRank, tagWorkerDone, MPI_COMM_WORLD, &flagDone, &status);
            if (flagDone > 0)
            {
                MPI_Recv(&workerDone, 1, MPI_INT, workerRank, tagWorkerDone, MPI_COMM_WORLD, &status);
                nbWorkersDone++;
                if ((worldSize-1) == nbWorkersDone)
                {
                    allWorkersDone = true;
                }
            }
        }
    }

}


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
        // VRM TODO: clarify ... use displayUsage() ?
        std::cout << "Usage: mpirun -np <nb of processes> -f <hostfile> " << argv[0] << " <param file>" << std::endl;
        return 1;
    }

    std::cout << "VRM: Launch " << argv[0] << " rank " << worldRank << std::endl;
    // Start EvaluatorControl on workers (not on master for now).
    if (0 != worldRank)
    {
        Evaluator evaluator;
        EvaluatorControl evc(evaluator);
        evc.run();
    }

    // The rest of the algo is in master only
    if (0 == worldRank)
    {
        int nbPoints = 100;
        if (argc >= 2)
        {
            nbPoints = std::atoi(argv[1]);
        }
        std::cout << "VRM: Generate points in rank " << worldRank << "... etc." << std::endl;
        std::vector<double> pointsVector = generatePoints(nbPoints);
        std::vector<EvalPoint> evalpointVector;
        std::cout << "Number of points to evaluate: " << nbPoints << std::endl;
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

    // Finalize the MPI environment.
    MPI_Finalize();
}

