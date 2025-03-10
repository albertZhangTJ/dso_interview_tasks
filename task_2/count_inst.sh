#!/bin/bash
mkdir build
cd build
cmake ..
make
cd ..
clang-14 -emit-llvm -c $1 -o test.bc
opt-14 -enable-new-pm=0 -load=$(pwd)/build/libInstCnt.so -instcnt < test.bc > /dev/null 2> result.txt
awk -f bin.awk result.txt > bin_counts.txt
gnuplot gp.gp
rm result.txt
rm bin_counts.txt
rm -rf build
rm test.bc