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


