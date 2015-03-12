/* Solution for the queens problem (revised):
 * https://www.hackerrank.com/challenges/queens-revised
 * 
 * Does fairly slow backtracking. Works for board sizes up to 64. n = 32 takes
 * about 5 s on an Intel i5.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

/* Results printing. */
#define PRINT_RESULT_BOARD 1
#define PRINT_RESULT       1

/* Revised queens problem. When off, only checks for conflicts on rows,
 * columns and diagonals. */
#define QUEENS_REVISED     1

/* Helper functions */
#define min(x,y) (x < y ? x : y)
#define findfirstset(x) (__builtin_ffs(x))

/* Type definitions.
 * Set maximum board size to one of 8, 16, 32, or 64. */

#define BOARD_SIZE_MAX 64

#if BOARD_SIZE_MAX == 64
#define row_t uint64_t
#endif

#if BOARD_SIZE_MAX == 32
#define row_t uint32_t
#endif

#if BOARD_SIZE_MAX == 16
#define row_t uint16_t
#endif

#if BOARD_SIZE_MAX == 8
#define row_t uint8_t
#endif

/* Forward declarations. */
void queens_revised(int n);
int  queens_recurse(row_t ** board, row_t ** illegal, int n, int i);
int  is_legal(row_t * illegal, int row, int n, row_t row_pos_bits);
void set_illegal(row_t * board, row_t * illegal, int row, int n,
        row_t row_pos_bits, int row_pos);

void print_bits(row_t x, int n);
void print_bitarray(row_t * X, int n);
void print_result(row_t * X, int n);

void copy(row_t * dest, row_t * src, int n);
void zero(row_t * X, int n);

/* Main program. */
int main(int argc, char * argv[])
{
    int n;
    if (argc == 2) {
        /* No error handling. */
        n = atoi(argv[1]);
    } else {
        printf("Usage: queens-revised n\n");
        return 1;
    }

    queens_revised(n);
    return 0;
}

/* Main functionality ********************************************************/

/* Main queens function.
 *
 * Takes an board size n (up to 64) and prints the positions of n queens on a
 * chessboard (one per row) such that:
 * 1. No two queens are on the same row, column or diagonal.
 * 2. No three (or more) queens are on the same line, e.g. like this (o is a
 * queen): 
 *      . . . . . . .
 *      . o . . . . .
 *      . . . o . . .
 *      . . . . . o .
 *      . . . . . . .
 * 
 * Performs a backtracking search.
 * O(n!) time complexity
 * O(n³) memory complexity (bits)
 *      */
void queens_revised(int n)
{
    assert(n > 0);
    assert(n <= BOARD_SIZE_MAX);

    row_t ** board   = malloc(n*sizeof(row_t *));
    row_t ** illegal = malloc(n*sizeof(row_t *));

    for (int i = 0; i < n; i++) {
        board[i]   = malloc(n*sizeof(row_t));
        illegal[i] = malloc(n*sizeof(row_t));

        zero(board[i], n);
        zero(illegal[i], n);
    }

    int success = queens_recurse(board, illegal, n, 0);

#if PRINT_RESULT_BOARD
    if (success) print_bitarray(board[n - 1], n);
#endif

#if PRINT_RESULT
    if (success) {
        print_result(board[n - 1], n);
    } else {
        printf("no result\n");
    }
#endif

    for (int i = 0; i < n; i++) {
        free(board[i]);
        free(illegal[i]);
    }
    free(board);
    free(illegal);
}

/* Actual computation for finding positions of queens. */
int queens_recurse(row_t ** board,
                   row_t ** illegal,
                   int n,
                   int i)
{

    /* If i = n we have placed all queens successfully. */
    if (i == n) return 1;

    /* Otherwise we try to place a new queen. */
    for (int new_pos = 0; new_pos < n; new_pos++) {

        row_t new_pos_bits = ((row_t) 1) << new_pos;
        if (i == 0 || is_legal(illegal[i - 1], i, n, new_pos_bits)) {

            if (i > 0) {
                /* Take queen positions and illegal positions from previous row. */
                copy(board[i],   board[i - 1],   n);
                copy(illegal[i], illegal[i - 1], n);
            } else {
                /* Zero positions if we're on the first row. */
                zero(board[i],   n);
                zero(illegal[i], n);
            }

            /* Place a new queen. */
            board[i][i] = new_pos_bits;

            /* Set the illegal positions resulting from the new queen. */
            set_illegal(board[i], illegal[i], i, n, new_pos_bits, new_pos);

            /* Continue to next row. */
            int success = queens_recurse(board, illegal, n, i + 1);

            /* If successful, we stop. */
            if (success) return 1;
        }
    }
    return 0;
}

/* Check if position illegal */
inline int is_legal(row_t * illegal,
                    int i,
                    int n, 
                    row_t row_pos_bits)
{
    row_t row = illegal[i];
    return !(row & row_pos_bits);
}

/* Set the illegal positions based on new position. Only set illegal positions
 * caused by the addition of a queen on row i.
 * */
void set_illegal(row_t * board,
                 row_t * illegal,
                 int i_cur,
                 int n,
                 row_t new_pos_bits,
                 int new_pos)
{
    /* Set diagonals to illegal. */
    int left_steps = min(n - i_cur - 1, new_pos);
    for (int step = 1; step <= left_steps; step++) {
        illegal[i_cur + step] |= (new_pos_bits >> step);
    }

    int right_steps = min(n - i_cur - 1, n - new_pos - 1);
    for (int step = 1; step <= right_steps; step++) {
        illegal[i_cur + step] |= (new_pos_bits << step);
    }

    /* Set vertical to illegal (horizontal not needed) */
    for (int step = 1; step < n - i_cur; step++) {
        illegal[i_cur + step] |= new_pos_bits;
    }

#if QUEENS_REVISED
    /* Then set straight lines from previous queens as illegal. */
    for (int i_prev = 0; i_prev < i_cur; i_prev++) {

        int board_i_prev  = findfirstset(board[i_prev]);
        int board_i_cur   = findfirstset(board[i_cur]);
        int row_diff      = i_cur - i_prev;
        int col_diff      = board_i_prev - board_i_cur;
        row_t illegal_pos = new_pos_bits;

        for (int i_rem = i_cur + row_diff;
                (i_rem < n) && (illegal_pos != 0);
                i_rem += row_diff) {
            if (col_diff > 0) {
                illegal_pos >>= col_diff;
            } else {
                illegal_pos <<= -col_diff;
            }

            illegal[i_rem] |= illegal_pos;
        }
    }
#endif

    return;
}

/* Miscellaneous helper functions ********************************************/

/* Print bit representation. */
inline void print_bits(row_t x, int n)
{
    for (int i = 0; i < n; i++) {
        printf("%c", (x & 1) ? 'o' : '.');
        x >>= 1;
    }
    return;
}

/* Print bit array (assume square). */
void print_bitarray(row_t * X, int n) {
    for (int i = 0; i < n; i++) {
        print_bits(X[i], n);
        printf("\n");
    }
    return;
}

/* Copy bit array. */
void copy(row_t * dest, row_t * src, int n)
{
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }
    return;
}

/* Zero bit array. */
void zero(row_t * X, int n)
{
    for (int i = 0; i < n; i++) {
        X[i] = 0;
    }
}

/* Print result.
 * Prints n on a row, followed by a list of the positions of the queens on
 * each row. */
void print_result(row_t * X, int n)
{
    printf("%d\n", n);
    for (int i = 0; i < n; i++) {
        printf("%d ", findfirstset(X[i]));
    }
    printf("\n");
}
