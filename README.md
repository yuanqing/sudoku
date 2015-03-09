# sudoku [![Version](https://img.shields.io/badge/version-v0.1.0-orange.svg?style=flat)](https://github.com/yuanqing/sudoku/releases) [![Build Status](https://img.shields.io/travis/yuanqing/sudoku.svg?branch=master&style=flat)](https://travis-ci.org/yuanqing/sudoku)

> A Sudoku solver using [local search](http://en.wikipedia.org/wiki/Local_search_%28constraint_satisfaction%29), written in C.

## Usage

```
$ cat puzzles/given/001.txt
# eg1
5 3 _ _ 7 _ _ _ _
6 _ _ 1 9 5 _ _ _
_ 9 8 _ _ _ _ _ _
8 _ _ _ 6 _ _ _ 3
4 _ _ 8 _ 3 _ _ 1
7 _ _ _ 2 _ _ _ 6
_ 6 _ _ _ _ 2 8 _
_ _ _ 4 1 9 _ _ 5
_ _ _ _ 8 _ _ 7 9
$ gcc sudoku.c -o sudoku
$ ./sudoku < puzzles/given/001.txt
5 3 4 6 7 8 9 1 2
6 7 2 1 9 5 3 4 8
1 9 8 3 4 2 5 6 7
8 5 9 7 6 1 4 2 3
4 2 6 8 5 3 7 9 1
7 1 3 9 2 4 8 5 6
9 6 1 5 3 7 2 8 4
2 8 7 4 1 9 6 3 5
3 4 5 2 8 6 1 7 9
```

- The puzzle is read in via `stdin`. A puzzle can contain comments; characters following a `#` are ignored.
- [A seed must be specified](#cli) if we want deterministic results when solving [puzzles with multiple solutions](http://sudopedia.enjoysudoku.com/Invalid_Test_Cases.html#Not_Unique_.E2.80.94_125_Solutions).
- Currently, because no checking is performed, the solver will *not* terminate if given an [unsolvable puzzle](http://sudopedia.enjoysudoku.com/Invalid_Test_Cases.html#Unsolvable_Square).

## CLI

```
usage: sudoku [-s seed]
```

- `seed` &mdash; An optional seed for the pseudo random number generator.

## Changelog

- 0.1.0
  - Initial release

## License

[MIT](https://github.com/yuanqing/sudoku/blob/master/LICENSE)
