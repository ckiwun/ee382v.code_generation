#! /bin/sh

rm -f *.ll *.bc ./input ./output
make clean
clang -emit-llvm -c sample_input/$1.c
cp $1.bc sample_input
opt -instnamer $1.bc | opt -mem2reg -o input.bc
llvm-dis input.bc -o input.ll
make
echo ----Live----
opt -load ./live.so -live input.bc -disable-output > sample_input/$1.live 2>&1
echo ----Reach----
opt -load ./reach.so -reach input.bc -disable-output > sample_input/$1.reach 2>&1
