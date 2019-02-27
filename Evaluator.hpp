#include <mpi.h>
#include <iostream>
#include <vector>


class Evaluator
{
public:
    Evaluator() {}

    // Mock evaluator.
    // Input: x.
    // Output: f.
    // Returns: true if eval went OK, false otherwise.
    bool eval_x(const double x, double &f);
};


