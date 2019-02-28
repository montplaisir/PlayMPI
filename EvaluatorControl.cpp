#include "EvaluatorControl.hpp"

void EvaluatorControl::run()
{
    // Get the rank of the process
    int workerRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &workerRank);
    // Useful values for debug.
    char processorName[MPI_MAX_PROCESSOR_NAME];
    int nameLen;
    MPI_Get_processor_name(processorName, &nameLen);


    std::cout << "VRM: run EvaluatorControl for rank " << workerRank << std::endl;

    // Start workers
    // Workers get a point, evaluate it, and return its evaluation.
    if (0 != workerRank)
    {
        bool evaluationDone = false;
        while (!evaluationDone)
        {
            double x = 0.0;
            double f = 0.0;
            if (getNewPointToEvaluate(x))
            {
                std::cout << "VRM: EvaluatorControl calls eval_x for rank " << workerRank << " on host " << processorName << std::endl;
                bool eval_ok = _evaluator.eval_x(x, f);
                sendPointToMaster(x, f, eval_ok);
            }
            evaluationDone = isEvaluationDone();
        }
    }
    // Send word that the worker is done.
    sendWorkerDoneToMaster();
}

bool EvaluatorControl::getNewPointToEvaluate(double &x)
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

void EvaluatorControl::sendPointToMaster(const double &x, const double &f, const double &eval_ok)
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

bool EvaluatorControl::isEvaluationDone()
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
void EvaluatorControl::sendWorkerDoneToMaster()
{
    int done = 1;
    MPI_Send(&done, 1, MPI_INT, 0, tagWorkerDone, MPI_COMM_WORLD);
}


