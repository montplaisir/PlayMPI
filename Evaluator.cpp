
#include "Evaluator.hpp"

bool Evaluator::eval_x(const double x, double &f)
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


