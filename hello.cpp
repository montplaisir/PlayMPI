#include <mpi.h>
#include <iostream>
#include <vector>


const int tagPointToEvaluate = 0;
const int tagEvaluatedPoint = 1;
const int tagEvaluationDone = 2;
const int tagWorkerDone = 3;

class EvalPoint
{
private:
    double   _x;
    int      _f;
    bool     _evalOk;
    int      _workerRank;

public:
    // Constructor
    EvalPoint(double x, double f, bool evalOk, int workerRank)
      : _x(x),
        _f(f),
        _evalOk(evalOk),
        _workerRank(workerRank)
    {}

    // Get/Set
    double getX()       { return _x; }
    int    getF()       { return _f; }
    double getEvalOk()  { return _evalOk; }
    int    getWorker()  { return _workerRank; }
};


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


// Master sends word to workers that evaluations are done.
void sendEvaluationDoneToWorkers(const int worldSize)
{
    int done = 1;
    for (int workerRank = 1; workerRank < worldSize; workerRank++)
    {
        MPI_Send(&done, 1, MPI_INT, workerRank, tagEvaluationDone, MPI_COMM_WORLD);
    }
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
    // Usage: mpirun -np <number of processes> -f <hostfile> hello

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
                bool eval_ok = eval_x(x, f);
                sendPointToMaster(x, f, eval_ok);
            }
            evaluationDone = isEvaluationDone();
        }
        // Send word that the worker is done.
        sendWorkerDoneToMaster();
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}

