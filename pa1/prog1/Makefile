CXXFLAGS = $(shell llvm-config --cxxflags) -fno-rtti

all: IRPrinter.so

IRPrinter.o: IRPrinter.cpp IRPrinter.h Makefile
	$(CXX) $(CXXFLAGS) IRPrinter.cpp -c -o $@
IRPrinter.so: IRPrinter.o
	$(CXX) -shared $^ -o $@
clean:
	rm -f *.o *.so
