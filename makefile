LAUNCH      = launch.exe
ALGO_EXE    = algo.exe

all: $(ALGO_EXE) $(LAUNCH)

#$(EXE): EvalPoint.hpp Evaluator.hpp EvaluatorControl.hpp evc.cpp
#	mpic++ -o $@ $?

Evaluator.o: Evaluator.cpp Evaluator.hpp
	mpic++ -c $< -o $@

EvaluatorControl.o: EvaluatorControl.cpp EvaluatorControl.hpp
	mpic++ -c $< -o $@

$(ALGO_EXE): Evaluator.o EvaluatorControl.o algo.cpp
	mpic++ -o $@ $?

$(LAUNCH): launch.cpp $(ALGO_EXE)
	g++ -o $@ $<

clean:
	rm -f $(ALGO_EXE) $(LAUNCH) *.o
