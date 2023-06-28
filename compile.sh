#!/bin/sh -x
gcc -O2 -o blur1 blur1.c
gcc -O2 -o blur2 blur2.c
gcc -O2 -fopenmp -o blurp blurp.c
