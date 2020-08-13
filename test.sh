#!/bin/bash

SIZES=''

run() {
  make clean
  make CC="$CC" CFLAGS="$CFLAGS"
  make CC="$CC" CFLAGS="$CFLAGS" test || {
    echo "Failed with $CC"
    exit 1;
  }
  SIZES+="$CC"$'\t'"$(du -b curskey.o)"$'\n'
}

CC=gcc     CFLAGS=""                 run
CC=clang   CFLAGS=""                 run
#CC=g++     CFLAGS="-Wold-style-cast" run
#CC=clang++ CFLAGS="-Wold-style-cast" run

echo "$SIZES"
