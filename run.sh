#!/bin/bash

# compile
gcc sudoku.c -o sudoku

# run
for puzzle in puzzles/*/*.txt; do
  echo $puzzle
  ./sudoku < $puzzle
done
