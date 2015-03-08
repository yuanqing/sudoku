#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>

#include "sudoku.h"

/**
 * Reads the `board` from STDIN.
 */
static void read_board() {
  char c;
  int in_comment = 0;
  int i = 0;
  while ((c = getchar()) != EOF) {
    //
    // Exit if we have read enough values.
    //
    if (i == SIZE) {
      return;
    }
    switch (c) {
    case '#':
      in_comment = 1;
      break;
    case '\n':
      in_comment = 0;
      break;
    case '\r':
    case ' ':
      break;
    default:
      //
      // If not in a comment, any non-space and non-digit is deemed an unknown.
      //
      if (!in_comment) {
        board[i / LEN][i % LEN] = isdigit(c) ? c - '0' : UNKNOWN;
        i++;
      }
    }
  }
}

/**
 * Prints `board` to STDOUT.
 */
static void print_board() {

  int i = -1;
  while (++i < LEN) {
    printf("%d", board[i][0]);
    int j = 0;
    while (++j < LEN) {
      printf(" %d", board[i][j]);
    }
    puts("");
  }

}

/**
 * Initialises the `unknowns` array.
 */
static void init_unknowns() {

  num_unknowns = 0;
  int i = -1;
  while (++i < LEN) {
    int j = -1;
    while (++j < LEN) {
      if (board[i][j] == UNKNOWN) {
        unknowns[num_unknowns++] = &board[i][j];
      }
    }
  }

}

/**
 * Initialises the `constraints` array.
 */
static void init_constraints() {

  int i = -1;
  while (++i < LEN) {
    int j = -1;
    while (++j < LEN) {
      constraint_t* row = &constraints[i];
      constraint_t* col = &constraints[LEN + j];
      constraint_t* sqr = &constraints[2 * LEN + SQR_INDEX[i][j]];
      if (board[i][j] == UNKNOWN) {
        row->index = i + 1;
        row->unknowns[row->num_unknowns++] = &board[i][j];
        col->index = j + 1;
        col->unknowns[col->num_unknowns++] = &board[i][j];
        sqr->index = SQR_INDEX[i][j] + 1;
        sqr->unknowns[sqr->num_unknowns++] = &board[i][j];
      } else {
        int mask = BIT_MASKS[board[i][j] - 1];
        //
        // Turn on the bit corresponding to the known value in
        // the `known_bit_mask`
        //
        row->known_bit_mask |= mask;
        col->known_bit_mask |= mask;
        sqr->known_bit_mask |= mask;
      }
    }
  }
  reset_constraints();

}

/**
 * Resets all constraints (including all unknown cells) to their initial state.
 */
static void reset_constraints() {

  int i = -1;
  while (++i < NUM_CONSTRAINTS) {
    constraint_t* c = &constraints[i];
    int j = -1;
    while (++j < c->num_unknowns) {
      *c->unknowns[j] = UNKNOWN;
      c->unsolved[j] = c->unknowns[j];
    }
    c->num_unsolved = c->num_unknowns;
    c->solved_bit_mask = c->known_bit_mask;
  }

}

/**
 * Recomputes the `num_unsolved` and `solved_bit_mask` of each constraint in
 * the `constraint` array, starting from the element at the given `startIndex`.
 */
static void update_constraints(int startIndex) {

  int i = startIndex - 1;
  while (++i < NUM_CONSTRAINTS) {
    constraint_t* c = &constraints[i];
    int j = -1;
    c->num_unsolved = 0;
    c->solved_bit_mask = c->known_bit_mask;
    while (++j < c->num_unknowns) {
      int val = *(c->unknowns)[j];
      if (val == UNKNOWN) {
        c->unsolved[c->num_unsolved++] = c->unknowns[j];
      } else {
        c->solved_bit_mask |= BIT_MASKS[val-1];
      }
    }
  }

}

/**
 * Sorts a subarray of the `constraint` array, starting from the element at the
 * given `startIndex`.
 */
static void sort_constraints(int startIndex) {

  qsort(constraints + startIndex, NUM_CONSTRAINTS - startIndex,
      sizeof(constraint_t), compare_constraints);

}

/**
 * Comparison function for sorting the `constraints` array in ascending order
 * of each constraint's number of unsolved cells.
 */
static int compare_constraints(const void* x, const void* y) {

  constraint_t* c1 = (constraint_t*) x;
  constraint_t* c2 = (constraint_t*) y;

  //
  // Return a negative integer when the constraint with a smaller `num_unsolved`
  // is to be ordered before the other.
  //
  int num_unsolved = c1->num_unsolved - c2->num_unsolved;
  if (num_unsolved != 0) {
    return num_unsolved;
  }

  //
  // Preserve the relative ordering of `c1` and `c2` if they have the same
  // `num_unsolved`.
  //
  return c1->index - c2->index;

}

/**
 * Try to solve the constraint at the given `index` of the `constraints` array.
 * Returns one of:
 * - SAT (the entire board, including the constraint at `index`, is solved),
 * - INCOMPLETE (the constraint was solved but the board still has other
 *   unsolved cells), or
 * - UNSAT (the constraint was not solved after `MAX_TRIES_PER_CONSTRAINT`).
 */
static int solve_constraint(int index) {

  constraint_t* c = &constraints[index];

  //
  // Exit if the constraint is already solved.
  //
  if (c->num_unsolved == 0) {
    return check_board();
  }

  //
  // Determine the set of possible values for the unsolved cells using the
  // contraint's `solved_bit_mask`.
  //
  int domain[9];
  get_domain(domain, c->solved_bit_mask);

  //
  // Try to solve this constraint using values in `domain`.
  //
  int i = -1;
  int tries = MAX_TRIES_PER_CONSTRAINT[c->num_unsolved - 1];
  while (++i < tries) {
    shuffle(domain, c->num_unsolved);
    int j = -1;
    while (++j < c->num_unsolved) {
      *(c->unsolved)[j] = domain[j];
    }
    //
    // Stop trying if the current solution for `unknowns` is either:
    // - SAT, or
    // - INCOMPLETE and not in the `tabu_list`.
    //
    int result = check_board();
    if (result != UNSAT && !is_tabu()) {
      return result;
    }
  }

  //
  // No solution found after `MAX_TRIES_PER_CONSTRAINT`. Reset the board
  // to its initial state before the attempt to solve this constraint,
  // ie. set all of this constraint's unsolved cells to `UNKNOWN`.
  //
  i = -1;
  while (++i < c->num_unsolved) {
    *(c->unsolved)[i] = UNKNOWN;
  }
  return UNSAT;

}

/**
 * Assign the possible values for the unsolved cells to the specified `domain`
 * array using the `solved_bit_mask`. Examples:
 * - 0b111111110 gives domain { 1 };
 * - 0b010111111 gives domain { 7, 9 }.
 */
static void get_domain(int* domain, int solved_bit_mask) {

  int i = -1;
  while (++i < LEN) {
    int mask = BIT_MASKS[i];
    if ((solved_bit_mask & mask) != mask) {
      *domain = i + 1;
      domain++;
    }
  }

}

/**
 * Shuffles the given `arr` of length `len`.
 */
static void shuffle(int* arr, int len) {

  while (len > 1) {
    len--;
    //
    // Swap `arr[len]` with `arr[r]`, where `r` is in the range [0, `len`].
    //
    int r = rand() % (len + 1);
    int* last = arr + len;
    int* swap = arr + r;
    int temp = *last;
    *last = *swap;
    *swap = temp;
  }

}

/**
 * Returns 1 if `unknowns` is in `tabu_list`, else returns 0.
 */
static int is_tabu() {

  int i = -1;
  while (++i < num_tabu) {
    int is_tabu = 1;
    int j = -1;
    while (++j < num_unknowns) {
      if (tabu_list[i][j] != *unknowns[j]) {
        is_tabu = 0;
        break;
      }
    }
    if (is_tabu) {
      return 1;
    }
  }
  return 0;

}

/**
 * Add the current solution for the unknowns (ie. values in the `unknowns`
 * array) to the `tabu_list`.
 */
static void set_tabu() {

  //
  // Exit if the solution is already in `tabu_list`.
  //
  if (is_tabu()) {
    return;
  }

  //
  // Copy `unknowns` into the `next_tabu` row of `tabu_list`.
  //
  int i = -1;
  while (++i < num_unknowns) {
    tabu_list[next_tabu][i] = *unknowns[i];
  }

  //
  // Increment `num_tabu` and `next_tabu`. The latter wraps around to 0 if
  // it reaches `MAX_TABU`.
  //
  if (num_tabu < MAX_TABU) {
    num_tabu++;
  }
  next_tabu++;
  if (next_tabu == MAX_TABU) {
    next_tabu = 0;
  }

}

/**
 * Reset `tabu_list` and `num_tabu`.
 */
static void reset_tabu_list() {

  int i = -1;
  while (++i < num_tabu) {
    int j = -1;
    while (++j < num_unknowns) {
      tabu_list[i][j] = 0;
    }
  }
  num_tabu = 0;

}

/**
 * Determine if the state of Sudoku `board` is one of:
 * - SAT (satisfies all rules with no unsolved cells), or
 * - UNSAT (violates at least one rule), or
 * - INCOMPLETE (board has some unsolved cells, but the other solved cells do
 *   not violate rules).
 */
static int check_board() {

  //
  // The `rows`, `cols` and `sqrs` arrays record the values used in each
  // constraint. Examples:
  // - rows[0] having the value 0b111111110 means that the value 1 has not been
  //   used in the 1st row of the board;
  // - cols[3] having the value 0b010111111 means that the values 7 and 9 have
  //   not been used in the 4th column of the board.
  //
  int rows[LEN], cols[LEN], sqrs[LEN];
  int i = -1;
  while (++i < LEN) {
    rows[i] = 0;
    cols[i] = 0;
    sqrs[i] = 0;
  }

  int result = SAT;
  i = -1;
  while (++i < LEN) {
    int j = -1;
    while (++j < LEN) {
      int val = board[i][j];
      if (val == UNKNOWN) {
        result = INCOMPLETE;
      } else {
        int mask = BIT_MASKS[val-1];
        int k = SQR_INDEX[i][j];
        //
        // Exit with UNSAT if `val` has already been used in the row, or
        // column, or square.
        //
        if ((rows[i] & mask) == mask ||
            (cols[j] & mask) == mask ||
            (sqrs[k] & mask) == mask) {
          return UNSAT;
        }
        //
        // Otherwise, mark `val` as used by turning on the corresponding bit.
        //
        rows[i] |= mask;
        cols[j] |= mask;
        sqrs[k] |= mask;
      }
    }
  }
  return result;

}

/**
 * Try to solve the board.
 */
static void solve_board() {

  //
  // Exit if the board is already UNSAT or SAT.
  //
  int result = check_board();
  if (result != INCOMPLETE) {
    return;
  }

  //
  // Try to solve each constraint.
  //
  sort_constraints(0);
  int num_tries = 0;
  int i = -1;
  while (++i < NUM_CONSTRAINTS) {
    int result = solve_constraint(i);
    if (result == SAT) {
      return;
    } else if (result == UNSAT) {
      num_tries++;
      if (num_tries == MAX_TRIES) {
        reset_tabu_list();
        num_tries = 0;
      } else {
        set_tabu();
      }
      reset_constraints();
      sort_constraints(0);
      i = -1;
    } else {
      update_constraints(i);
      sort_constraints(i + 1);
    }
  }

}

/**
 * The main program.
 */
int main(int argc, char** argv) {

  //
  // Parse command line arguments.
  //
  long seed = time(NULL);
  if (argc > 1) {
    char* prog = *argv;
    argv++;
    //
    // Print usage message if option is "-h", "-help", or "--help".
    //
    if (strstr(*argv, "-h") != NULL) {
      fprintf(stderr, "usage: %s [-s seed]\n", prog);
      return 1;
    }
    //
    // Exit if unknown option, or option without argument, or invalid seed.
    //
    if (strcmp(*argv, "-s") != 0) {
      fprintf(stderr, "%s: unknown option %s\n", prog, *argv);
      return 1;
    }
    if (argc == 2) {
      fprintf(stderr, "%s: option %s requires an argument\n", prog, *argv);
      return 1;
    }
    errno = 0;
    seed = strtol(argv[1], argv, 10);
    if (errno != 0 || seed > UINT_MAX) {
      fprintf(stderr, "%s: invalid seed -- %s\n", prog, *argv);
      return 1;
    }
  }

  //
  // Initialise the random number generator with the `seed`.
  //
  srand((unsigned int) seed);

  //
  // Read in and initialise the board.
  //
  read_board();
  init_unknowns();
  init_constraints();

  //
  // Solve, and print the solved board.
  //
  solve_board();
  print_board();

  return 0;

}
