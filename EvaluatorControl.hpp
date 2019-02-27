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

    void run()
    {
        // Get the rank of the process
        int worldRank;
        MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

        std::cout << "VRM: run EvaluatorControl for rank " << worldRank << std::endl;

        // Start workers
        // Workers get a point, evaluate it, and return its evaluation.
        if (0 != worldRank)
        {
            bool evaluationDone = false;
            while (!evaluationDone)
            {
                double x = 0.0;
                double f = 0.0;
                if (getNewPointToEvaluate(x))
                {
                    std::cout << "VRM: EvaluatorControl calls eval_x for rank " << worldRank << std::endl;
                    bool eval_ok = _evaluator.eval_x(x, f);
                    sendPointToMaster(x, f, eval_ok);
                }
                evaluationDone = isEvaluationDone();
            }
        }
        // Send word that the worker is done.
        sendWorkerDoneToMaster();
    }

    bool getNewPointToEvaluate(double &x)
    {
        // Probe if there is a Send from master waiting to be received by this worker.
    
        bool newPointReceived = false;
        MPI_Status status;
        int flagNewPointToEval = 0;

        MPI_Iprobe(0, tagPointToEvaluate, MPI_COMM_WORLD, &flagNewPointToEval, &status);

        if (flagNewPointToEval > 0)
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
            MPI_Request request;
            MPI_Recv(&x, 1, MPI_DOUBLE, 0, tagPointToEvaluate, MPI_COMM_WORLD, &status);
   
            newPointReceived = true;
        }

        return newPointReceived;
    }

    void sendPointToMaster(const double &x, const double &f, const double &eval_ok)
    {
        // Send back evaluation to master
        // Although x is not new info, sending it back in the same message along with f and eval_ok
        // seems easier to manage.
        // Sending as 3 double for simplicity.
        double evalpoint[3];
        evalpoint[0] = x;
        evalpoint[1] = f;
        evalpoint[2] = eval_ok;

        MPI_Send(&evalpoint, 3, MPI_DOUBLE, 0, tagEvaluatedPoint, MPI_COMM_WORLD);
    }

    bool isEvaluationDone()
    {
        bool retDone = false;
        int evaluationDone = 0;
        int flagEvaluationDone = 0;
        MPI_Status status;
        MPI_Iprobe(0, tagEvaluationDone, MPI_COMM_WORLD, &flagEvaluationDone, &status);
        if (flagEvaluationDone > 0)
        {
            MPI_Recv(&evaluationDone, 1, MPI_INT, 0, tagEvaluationDone, MPI_COMM_WORLD, &status);
            retDone = true;
        }
        return retDone;
    }

    // Worker sends word to master that it is done.
    void sendWorkerDoneToMaster()
    {
        /*
        // Useful values for debug.
        int workerRank;
        MPI_Comm_rank(MPI_COMM_WORLD, &workerRank);
        char processorName[MPI_MAX_PROCESSOR_NAME];
        int nameLen;
        MPI_Get_processor_name(processorName, &nameLen);
        std::cout << "Worker " << workerRank << " on processor " << processorName << " is done." << std::endl;
        */

        int done = 1;
        MPI_Send(&done, 1, MPI_INT, 0, tagWorkerDone, MPI_COMM_WORLD);
    }

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
