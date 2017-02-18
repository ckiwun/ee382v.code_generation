CXXFLAGS = $(shell llvm-config --cxxflags) -fno-rtti

all: instrument.so

InstrumentPass.o: InstrumentPass.cpp InstrumentPass.h Makefile
	$(CXX) $(CXXFLAGS) InstrumentPass.cpp -c -o $@

instrument.so: InstrumentPass.o
	$(CXX) -shared $^ -o $@

clean:
	rm -f *.o *~ *.so *.ll *.bc *.out ready/*
