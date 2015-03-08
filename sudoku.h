#ifndef SUDOKU_H
#define SUDOKU_H

#define LEN 9
#define SIZE LEN * LEN
#define NUM_CONSTRAINTS LEN * 3
#define UNKNOWN 0
#define SAT 1
#define UNSAT 0
#define INCOMPLETE -1
#define MAX_TRIES 1000
#define MAX_TABU 100

typedef struct {
  int  index;
  char *unknowns[LEN];
  int  num_unknowns;
  char *unsolved[LEN];
  int  num_unsolved;
  int  known_bit_mask;
  int  solved_bit_mask;
} constraint_t;

int MAX_TRIES_PER_CONSTRAINT[LEN] = {
  1,
  20,
  50,
  100,
  500,
  1000,
  10000,
  50000,
  100000
};

int BIT_MASKS[LEN] = {
  0b000000001,
  0b000000010,
  0b000000100,
  0b000001000,
  0b000010000,
  0b000100000,
  0b001000000,
  0b010000000,
  0b100000000
};

int BIT_MASKS_FULL[LEN] = {
  0b000000001,
  0b000000011,
  0b000000111,
  0b000001111,
  0b000011111,
  0b000111111,
  0b001111111,
  0b011111111,
  0b111111111
};

/* to speed up square index lookup from a given (i, j) coordinate */
int SQR_INDEX[LEN][LEN] = {
  { 0, 0, 0, 1, 1, 1, 2, 2, 2 },
  { 0, 0, 0, 1, 1, 1, 2, 2, 2 },
  { 0, 0, 0, 1, 1, 1, 2, 2, 2 },
  { 3, 3, 3, 4, 4, 4, 5, 5, 5 },
  { 3, 3, 3, 4, 4, 4, 5, 5, 5 },
  { 3, 3, 3, 4, 4, 4, 5, 5, 5 },
  { 6, 6, 6, 7, 7, 7, 8, 8, 8 },
  { 6, 6, 6, 7, 7, 7, 8, 8, 8 },
  { 6, 6, 6, 7, 7, 7, 8, 8, 8 }
};

char board[LEN][LEN];
constraint_t constraints[NUM_CONSTRAINTS];
char *unknowns[SIZE];
int  num_unknowns;
char tabu_list[MAX_TABU][SIZE];
int  num_tabu;
int  next_tabu;

static void read_board();
static void print_board();
static void init_unknowns();
static void init_constraints();
static void reset_constraints();
static void update_constraints(int);
static void sort_constraints();
static int  compare_constraints();
static int  solve_constraint(int);
static void get_domain(int*, int);
static void shuffle(int*, int);
static int  is_tabu();
static void set_tabu();
static void reset_tabu_list();
static void solve_board();
static int  check_board();

#endif
