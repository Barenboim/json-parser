#!/bin/bash -eu

$CC $CFLAGS -c rbtree.c -o rbtree.o 
$CC $CFLAGS -c json_parser.c -o json_parser.o 

# Copy fuzzer executable to $OUT/
$CC $CFLAGS $LIB_FUZZING_ENGINE \
  .clusterfuzzlite/json_parse_fuzzer.c \
  -o $OUT/json_parse_fuzzer \
  json_parser.o rbtree.o -I$PWD/
