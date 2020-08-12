#!/bin/bash

SIZES=''

for compiler in gcc clang g++ clang++; do
  make clean
  make CC=$compiler
  make CC=$compiler test || {
    echo "Failed with $CC"
    exit 1;
  }
  SIZES+="$compiler"$'\t'"$(du -b curskey.o)"$'\n'
done

echo "$SIZES"
