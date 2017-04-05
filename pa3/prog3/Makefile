CXXFLAGS = $(shell llvm-config --cxxflags) -fno-rtti

all: live.so reach.so

COMMON = dataflow.cpp dataflow.h DataFlowAnnotator.h

live.o: live.cpp live.h $(COMMON) Makefile
	$(CXX) $(CXXFLAGS) live.cpp -c -o $@

reach.o: reach.cpp reach.h $(COMMON) Makefile
	$(CXX) $(CXXFLAGS) reach.cpp -c -o $@

live.so: live.o
	$(CXX) -shared $^ -o $@

reach.so: reach.o
	$(CXX) -shared $^ -o $@

clean:
	rm -f *.o *~ *.so *.ll *.bc *.out ready/*
