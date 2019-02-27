#include <mpi.h>
#include <iostream>
#include <vector>

#include "Evaluator.hpp"

const int tagPointToEvaluate = 0;
const int tagEvaluatedPoint = 1;
const int tagEvaluationDone = 2;
const int tagWorkerDone = 3;

class EvaluatorControl
{
    Evaluator _evaluator;
public:
    // Constructor
    EvaluatorControl(Evaluator evaluator)
      : _evaluator(evaluator)
    {}

    void run();

    bool getNewPointToEvaluate(double &x);

    void sendPointToMaster(const double &x, const double &f, const double &eval_ok);

    bool isEvaluationDone();

    // Worker sends word to master that it is done.
    void sendWorkerDoneToMaster();

};


/*

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
*/
