#include <mpi.h>
#include <iostream>
#include <vector>


class EvalPoint
{
private:
    double  _x;
    int     _f;
    bool    _evalOk;
    int     _workerRank;

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

