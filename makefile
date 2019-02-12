LAUNCH  = launch.exe
EXE     = hello.exe

all: $(EXE) $(LAUNCH)

$(EXE): hello.cpp
	mpic++ -o $@ $<

$(LAUNCH): launch.cpp hello.exe
	g++ -o $@ $<

clean:
	rm -f $(EXE) $(LAUNCH)
