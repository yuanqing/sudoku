#!/bin/bash

# compile
gcc sudoku.c -Wall -o sudoku

# run
for puzzle in puzzles/*/*.txt; do
  echo $puzzle
  ./sudoku < $puzzle
done
