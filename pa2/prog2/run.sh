#! /bin/sh

# Run: ./run.sh simpleloop

mkdir -p ready
make clean && make
clang++ test/$1.cc -I. -c -emit-llvm -O0
opt -dce -mem2reg -break-crit-edges -loop-simplify $1.bc -o ready/$1.bc

opt -load ./instrument.so -epp ready/$1.bc -o output.bc > path-desc.ll
# llvm-dis output.bc -o - | less

g++ --std=c++11 -c epp_runtime.cpp -o epp_runtime.o
ar rcs libepp_runtime.a epp_runtime.o
clang++ output.bc ./libepp_runtime.a -o a.out
./a.out 10 2> path-prof.ll
cat *.ll

