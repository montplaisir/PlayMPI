EXE = hello.exe

all: $(EXE)

$(EXE): hello.cpp
	mpic++ -o $@ $<

clean:
	rm -f $(EXE)
