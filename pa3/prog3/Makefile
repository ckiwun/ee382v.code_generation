CXXFLAGS = -w $(shell llvm-config --cxxflags) -fno-rtti

all: live.so reach.so
reach: reach.so
live: live.so

.PHONY: live reach clean

live.o: live.cpp live.h dataflow.h Makefile
	$(CXX) $(CXXFLAGS) live.cpp -c -o $@
reach.o: reach.cpp reach.h dataflow.h Makefile
	$(CXX) $(CXXFLAGS) reach.cpp -c -o $@
#dataflow.o: dataflow.h Makefile
#	$(CXX) $(CXXFLAGS) dataflow.h -c -o $@
#util.o: util.h Makefile
#	$(CXX) $(CXXFLAGS) util.h -c -o $@
#meet.o: meet.h Makefile
#	$(CXX) $(CXXFLAGS) meet.h -c -o $@
#transfer.o: transfer.h Makefile
#	$(CXX) $(CXXFLAGS) transfer.h -c -o $@

live.so: live.o 
	$(CXX) -shared $^ -o $@
reach.so: reach.o 
	$(CXX) -shared $^ -o $@

clean:
	rm -f *.o *~ *.so
