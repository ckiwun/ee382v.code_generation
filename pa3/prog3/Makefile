CXXFLAGS = $(shell llvm-config --cxxflags) -fno-rtti

all: live.so reach.so

dataflow.o: dataflow.cpp dataflow.h Makefile
	$(CXX) $(CXXFLAGS) dataflow.cpp -c -o $@

live.o: live.cpp live.h Makefile
	$(CXX) $(CXXFLAGS) live.cpp -c -o $@

reach.o: reach.cpp reach.h Makefile
	$(CXX) $(CXXFLAGS) reach.cpp -c -o $@

live.so: dataflow.o live.o 
	$(CXX) -shared $^ -o $@

reach.so: reach.o dataflow.o
	$(CXX) -shared $^ -o $@

clean:
	rm -f *.o *~ *.so *.ll *.bc *.out ready/*
