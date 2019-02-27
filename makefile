LAUNCH      = launch.exe
ALGO_EXE    = algo.exe

all: $(ALGO_EXE) $(LAUNCH)

#$(EXE): EvalPoint.hpp Evaluator.hpp EvaluatorControl.hpp evc.cpp
#	mpic++ -o $@ $?

EvaluatorControl.o: EvaluatorControl.hpp
	mpic++ -c $@ $?

$(ALGO_EXE): EvaluatorControl.o algo.cpp
	mpic++ -o $@ $?

$(LAUNCH): launch.cpp $(ALGO_EXE)
	g++ -o $@ $<

clean:
	rm -f $(ALGO_EXE) $(LAUNCH) *.o
