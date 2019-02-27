#include <mpi.h>
#include <iostream>
#include <vector>


class Evaluator
{
public:
    Evaluator();

    // Mock evaluator.
    // Input: x.
    // Output: f.
    // Returns: true if eval went OK, false otherwise.
    bool eval_x(const double x, double &f)
    {
        // Debug
        /*
        int workerRank;
        MPI_Comm_rank(MPI_COMM_WORLD, &workerRank);
        char processorName[MPI_MAX_PROCESSOR_NAME];
        int nameLen;
        MPI_Get_processor_name(processorName, &nameLen);
        std::cout << "Worker " << workerRank << " on processor " << processorName << " is doing an evaluation." << std::endl;
        */

        bool eval_ok = false;

        f = static_cast<int> (x);
        eval_ok = true;

        return eval_ok;
    }
};


/*
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
