/*
 * C language Sudoku solver
 *
 * Steve Jacobson, 01/05/06
 *
 * Copyright (c) 1996-2018 by Steven A. Jacobson, all rights reserved.
 */

/*
 * A standard Sudoku board contains 9 x 9 = 81 cells.  After the board is
 * solved, each cell can contain exactly one value (number).  In a 9 x 9
 * board, that value can be one of 1 - 9, inclusive.
 *
 * A determined cell represents (contains) exactly one value.
 * An undetermined cell can contain one of up to 9 possible values.
 * Before a board is solved, the board contains a mix of determined and
 * undetermined cells.  After solution, the board must contain all
 * determined cells.
 *
 * Each cell exists in exactly three dimensions; a row, a column and
 * a square.  Each dimension in a standard game contains 9 cells.
 *
 * Here is a summary of the algorithms used by the algorithmic solver:
 *
 * Cannot be analysis:  Within a dimension, there may be one or more
 *     determined cells, each containing one specific value.  Those
 *     specific values are eliminated as possible values from all
 *     undetermined cells in that dimension.
 *
 * Must be analysis:  Within a dimension, if exactly one cell contains 
 *     a particular value, then that cell must represent that value.
 *     All other possible values in that cell are cleared.  An
 *     undetermined cell meeting this condition becomes a determined cell.
 *
 * Two must be analysis:
 * 2 common 2 analysis (unimplemented):  A case of must be analysis.  
 *     Within a dimension, if two cells both contain the same two values
 *     which appear in no other cells in the dimension, then all other 
 *     possible values in those two cells can be cleared.
 *
 * N must be analysis:
 * n common n analysis (unimplemented):  General case of 2 common 2.
 *     Within a dimension, if n cells all contain the same n values
 *     which appear in no other cells in the dimension, then all other 
 *     possible values in those cells are cleared.
 *
 * Pair analysis:  Within a dimension, if two cells containing a matching
 *     pair exist, then both values in the pairs are "spoken for."  (The
 *     two cells must contain only the matching pair, and no other values.)
 *     Therefore, those two values cannot exist in any other cell in that
 *     dimension and are cleared.
 *
 *     Example:
 *       |   6  |  34  |   9  | 2348 |   7  |  34  |  235 |  123 | 1358 |
 *     becomes:
 *       |   6  |  34  |   9  | 2  8 |   7  |  34  |  2 5 |  12  | 1 58 |
 *
 *     In the example, the second and sixth cells contain "34".  This means
 *     both cells must contain either a 3 or a 4.  Ultimately, one cell will
 *     contain a 3 and the other a 4.  Since both 3 and 4 are unquestionably
 *     spoken for, the values 3 and 4 can be eliminated from all other cells
 *     in this dimension.
 *
 * Modified tuple analysis:
 *     this does subtuple analysis 1:  236 23 36
 *     this does subtuple analysis 2:  236 236 36
 *     should work for (not tested)    236 236 236
 *     also works for other variants:
 *       shown to work for:            789 789 8
 *
 *     Example:
 *       |  236 | 23678|   5  | 13678|   9  |  36  |   4  |  23  |  16  |
 *     becomes:
 *       |  236 |  78  |   5  |  178 |   9  |  36  |   4  |  23  |   1  |
 *
 * Serpentine analysis:  Within a dimension, if a number of cells contain
 *     a serpentine, as shown in the examples below, all of the values in
 *     the serpentine cells are "spoken for."  Therefore, all values
 *     contained in the serpentine cells can be cleared from the balance
 *     of the cells in the dimension.
 *
 *     at present, all serpentines involve exactly three cells.
 *
 *     symbolic example:  AB BC AC      concrete example:  12 23 13
 *     symbolic example:  AB BC ABC     concrete example:  12 23 123
 *     symbolic example:  AB ABC ABC    concrete example:  12 123 123
 *     symbolic example:  ABC ABC ABC   concrete example:  123 123 123
 *
 *   Example:  perfect serpentine:  56 25 26
 *    |   56  |  4    | 3     | 125 9 | 125 9 | 2  5  |    7  |    8  | 2  6  |
 *   becomes:
 *    |   56  |  4    | 3     | 1   9 | 1   9 | 2  5  |    7  |    8  | 2  6  |
 *
 *   Example:  serpentine:  278 27 78
 *    | 24789 | 2 78  | 1     |   5   | 2  7  |  3    |    6  |    78 | 4 789 |
 *   becomes:
 *    |  4  9 | 2 78  | 1     |   5   | 2  7  |  3    |    6  |    78 | 4   9 |
 *
 *   Example:  serpentine:  46 146 146
 *    | 34 67 |  4 6  | 34 67 |     9 | 1 4 6 |   5   | 2     | 1 4 6 |    8  |
 *   becomes:
 *    | 3   7 |  4 6  | 3   7 |     9 | 1 4 6 |   5   | 2     | 1 4 6 |    8  |
 *
 *   Example:  serpentine:  568 568 568
 *    |  4    | 1     | 25 78 |     9 |  3    |  56 8 |  56 8 |  56 8 | 2 678 |
 *   becomes:
 *    |  4    | 1     | 2  7  |     9 |  3    |  56 8 |  56 8 |  56 8 | 2  7  |
 *
 * (multi) dimensional analysis
 *     Within a square dimension, two or three undetermined cells may be
 *     the only cells which can possibly contain a particular value.  If
 *     those cells are also fully contained in another dimension (a row
 *     or a column), then the value is cleared in all other cells of that
 *     other dimension.
 *     Example:
 *       In the rightmost square, 369 and 69 represent the only possible 9s
 *       in that square.  One of them must contain 9.  Since both 369 and 69
 *       are also in the same row dimension, no other value in that row
 *       can contain 9.  So, 369 in the first column can be set to 36 
 * 
 *       | 3 69 | 2    | 1    |   5  |    7 |  4   |    8 | 3 69 |   69 |
 *       | 3 89 |   5  | 3 89 | 3  8 |   6  | 1    |  4   |   7  | 2    |
 *       |   7  |  4 8 | 3468 | 3  8 |    9 | 2    |   5  | 3 6  | 1    |
 *     becomes:
 *       | 3 6  | 2    | 1    |   5  |    7 |  4   |    8 | 3 69 |   69 |
 *       | 3 89 |   5  | 3 89 | 3  8 |   6  | 1    |  4   |   7  | 2    |
 *       |   7  |  4 8 | 3468 | 3  8 |    9 | 2    |   5  | 3 6  | 1    |
 *
 */

static const char rcsid[]="$Id: sudoku.c,v 1.89 2018/06/23 08:33:04 stevej Exp $";

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "sudoku.h"
#include "sudoku_io.h"
#include "sudoku_print.h"
#include "sudoku_printd.h"


/*
 * Globals
 */
dprint_t   *g_dprint = NULL;


/*
 * Forward declarations
 */
static bool cell_is_marked(cell_t *cell);
static int get_relevant_cells(board_t *board, uint4 input_row, uint4 input_col,
                              uint4 dimension, cell_t **return_array);
static uint4 get_lowest_cell_value(cell_t *cell);
void board_set_pointers(board_t *board, game_t *game, board_t *prev,
                        board_t *next);
bool cell_value_set(cell_t *cell, uint4 value);


#ifdef REMOVED
/*
 * Print the numerical representation of each TRUE value in a
 * values array.  This array is as that contained in a cell_t.
 * Note that for a standard 9x9 Sudoku game, array indices
 * 0-8 represent values 1-9.
 */
void
print_array(bool array[DIMENSION_SIZE])
{
    uint4    i;

    for (i=0; i < DIMENSION_SIZE; i++) {
        if (array[i]) {
            printf("%u", i + 1);
        } else {
            printf(" ");
        }
    }
}

/*
 * Print the numerical representation of each TRUE value in a
 * cell values array.
 */
void
print_cell_cell_vals(cell_t *cell)
{
    print_array(cell->values);
}

/*
 * Print the numerical representation of each TRUE value in a
 * cell values array, framed for board representation.
 */
void
print_cell_cell(cell_t *cell)
{
    printf(" ");
    print_cell_cell_vals(cell);
    printf(" |");
}

/*
 * Print the numerical representation of each TRUE value in a
 * cell values array, framed for board representation.
 */
void
print_cell(board_t *board, uint4 row, uint4 col)
{
    cell_t    *cell;

    cell = &board->x_y_board[row][col];

    print_cell_cell(cell);
}

/*
 * Print a representation of a single dimension
 * Fixme:  Could be used in print_board()
 */
void
print_dimension(cell_t *cells[DIMENSION_SIZE])
{
    uint4    i;

    printf("|");
    for (i=0; i < DIMENSION_SIZE; i++) {
        print_cell_cell(cells[i]);
    }
    printf("\n");
}

/*
 * Print a representation of an entire game board.
 */
void
print_board(board_t *board)
{
    uint4    row, col;

    printf("=======================================================");
    printf("======================================================\n");
    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        printf("|");
        for (col=0; col < NUM_VERT_CELLS; col++) {
            print_cell(board, row, col);
        }
        printf("\n");
        if ((row % 3) == 2) {
            printf("=======================================================");
            printf("======================================================\n");
        }
    }
}
#endif  /* REMOVED */

/*
 * The board is solved if every cell in the game has exactly one
 * possible value.  Does NOT check to see if the cell values are sane.
 */
bool
board_is_solved(board_t *board)
{
    uint4     row;
    uint4     col;
    cell_t    *cell;

    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        for (col=0; col < NUM_VERT_CELLS; col++) {
            cell = &board->x_y_board[row][col];
            if (cell->num_possible_values != 1) {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}

/*
 * The value in any determined cell in this dimension cannot match the
 * value in any other determined cell in this dimension.
 * Makes no assumptions about, and does no checking of, the values
 * contained in undetermined cells.
 * Fixme:  Do not need to look backwards for determined cell checks.
 */
bool
test_sanity_of_dimension_never_solved(cell_t *test_cells[DIMENSION_SIZE])
{
    cell_t    *cell1;
    cell_t    *cell2;
    uint4     cell_index1;
    uint4     cell_index2;
    uint4     cell1_val;

    /*
     * For each determined cell, make sure the value it contains is not
     * contained in any other determined cell in this dimension.
     */
    for (cell_index1=0; cell_index1 < DIMENSION_SIZE; cell_index1++) {
        cell1 = test_cells[cell_index1];
        /*
         * If this is a determined cell, fail if any of the other
         * determined cells contain the value represented by this
         * determined cell.
         */
        if (1 == cell1->num_possible_values) {
            cell1_val = get_lowest_cell_value(cell1);

            for (cell_index2=0; cell_index2 < DIMENSION_SIZE; cell_index2++) {
                cell2 = test_cells[cell_index2];

                /* Fail if cell1 value is contained in cell2 */
                if ((cell1 != cell2) && (1 == cell2->num_possible_values) &&
                  (cell_value_set(cell2, cell1_val))) {
                    return (FALSE);
                }
            }
        }
    }

    return (TRUE);
}

/*
 * The value in any determined cell in this dimension cannot match the
 * value in any other determined cell in this dimension.
 * The value also must not match a value in any undetermined cell.
 * Requires that an attempt has already been made to solve the dimension.
 * In other words, determined cell values must have already been
 * eliminated from the undetermined cells.
 */
bool
test_sanity_of_dimension_part_solved(cell_t *test_cells[DIMENSION_SIZE])
{
    cell_t    *cell1;
    cell_t    *cell2;
    uint4      cell_index1;
    uint4      cell_index2;
    uint4      cell1_val;

    /*
     * For each determined cell, make sure the value it contains is not
     * contained in any other cell in this dimension.
     */
    for (cell_index1=0; cell_index1 < DIMENSION_SIZE; cell_index1++) {
        cell1 = test_cells[cell_index1];
        /*
         * If this is a determined cell, check if any of the other cells
         * contain the value represented by that determined cell.
         */
        if (1 == cell1->num_possible_values) {
            cell1_val = get_lowest_cell_value(cell1);

            for (cell_index2=0; cell_index2 < DIMENSION_SIZE; cell_index2++) {
                cell2 = test_cells[cell_index2];

                /* Fail if cell1 value is contained in cell2 */
                if ((cell1 != cell2) && (cell_value_set(cell2, cell1_val))) {
                    return (FALSE);
                }
            }
        }
    }

    return (TRUE);
}

/*
 * The dimension is sane in a solved context if each number 1-9
 * is represented exactly once.
 * Requires that the dimension has already been solved.
 */
bool
test_sanity_of_dimension_solved(cell_t *test_cells[DIMENSION_SIZE])
{
    cell_t    *cell;
    uint4      test_val;
    uint4      cell_val;
    uint4      num_test_vals;
    uint4      cell_index;

    /*
     * For each test value 1-9, make sure that it exists exactly once in
     * all the cells in this dimension.
     */
    for (test_val=1; test_val <= DIMENSION_SIZE; test_val++) {
        num_test_vals = 0;

        for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
            cell = test_cells[cell_index];

            /* This test cannot fail in a solved dimension */
            if (cell->num_possible_values != 1) {
                return (FALSE);
            }
            cell_val = get_lowest_cell_value(cell);
            if (cell_val == test_val) {
                num_test_vals++;
            }
        }
        if (num_test_vals != 1) {
            return (FALSE);
        }
    }

    return (TRUE);
}

/*
 * The game is sane in a solved context if, within every dimension,
 * each number 1-9 is represented exactly once.
 * Requires that every dimension has already been solved.
 */
bool
board_is_sane_solved(board_t *board)
{
    uint4     row = 101;
    uint4     col = 102;
    cell_t    *test_cells[DIMENSION_SIZE];
    bool rc;

    for (row=0; row < NUM_VERT_CELLS; row++) {
        /* Create a pointer array for the cells in each row dimension */
        get_relevant_cells(board, row, 0, ROW, test_cells);
        rc = test_sanity_of_dimension_solved(test_cells);
        if (!rc) {
            return (FALSE);
        }
    }

    for (col=0; col < NUM_HORIZ_CELLS; col++) {
        /* Create a pointer array for the cells in each column dimension */
        get_relevant_cells(board, 0, col, COL, test_cells);
        rc = test_sanity_of_dimension_solved(test_cells);
        if (!rc) {
            return (FALSE);
        }
    }

    for (row=0; row < NUM_VERT_CELLS; row += NUM_VERT_CELLS_IN_SQUARE) {
        for (col=0; col < NUM_HORIZ_CELLS; col += NUM_HORIZ_CELLS_IN_SQUARE) {
            /* Create a pointer array for the cells in each square dimension */
            get_relevant_cells(board, row, col, SQUARE, test_cells);
            rc = test_sanity_of_dimension_solved(test_cells);
            if (!rc) {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}

/*
 * The game is sane in a partially solved context if, within each
 * dimension:
 * - The value in any determined cell in the dimension cannot match
 *   the value in any other determined cell in the dimension.
 * - The value also must not match a value in any undetermined cell.
 * - Requires that an attempt has already been made to solve the
 *   dimension.  In other words, determined cell values must have
 *   already been eliminated from the undetermined cells.
 * - Also, the number of possible values contained in each cell
 *   must be from 0 to 9, inclusive.
 */
bool
board_is_sane_part_solved(board_t *board)
{
    cell_t    *test_cells[DIMENSION_SIZE];
    cell_t    *cell;
    uint4     row = 103;
    uint4     col = 104;
    bool      rc;

    /*
     * Range-check the sanity of cells.  In particular, look for any
     * cells that have undeflowed to 0 possible values.  This can
     * happen if a guess is wrong in recursive descent.
     */
    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        for (col=0; col < NUM_VERT_CELLS; col++) {
            cell = &board->x_y_board[row][col];
            if ((1 > cell->num_possible_values) ||
              (DIMENSION_SIZE < cell->num_possible_values)) {
                return (FALSE);
            }
        }
    }

    for (row=0; row < NUM_VERT_CELLS; row++) {
        /* Create a pointer array for the cells in each row dimension */
        get_relevant_cells(board, row, 0, ROW, test_cells);
        rc = test_sanity_of_dimension_part_solved(test_cells);
        if (!rc) {
            return (FALSE);
        }
    }

    for (col=0; col < NUM_HORIZ_CELLS; col++) {
        /* Create a pointer array for the cells in each column dimension */
        get_relevant_cells(board, 0, col, COL, test_cells);
        rc = test_sanity_of_dimension_part_solved(test_cells);
        if (!rc) {
            return (FALSE);
        }
    }

    for (row=0; row < NUM_VERT_CELLS; row += NUM_VERT_CELLS_IN_SQUARE) {
        for (col=0; col < NUM_HORIZ_CELLS; col += NUM_HORIZ_CELLS_IN_SQUARE) {
            /* Create a pointer array for the cells in each square dimension */
            get_relevant_cells(board, row, col, SQUARE, test_cells);
            rc = test_sanity_of_dimension_part_solved(test_cells);
            if (!rc) {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}

/*
 * The game is sane in an unsolved context if, within each dimension:
 * - The value in any determined cell in the dimension cannot match
 *   the value in any other determined cell in the dimension.
 * - Also, the number of possible values contained in each cell
 *   must be from 0 to 9, inclusive.
 * - Makes no assumptions about, and does no checking of, the values
 *   contained in undetermined cells.
 */
bool
board_is_sane_never_solved(board_t *board)
{
    cell_t    *test_cells[DIMENSION_SIZE];
    cell_t    *cell;
    uint4     row = 105;
    uint4     col = 106;
    bool      rc;

    /*
     * Range-check the sanity of cells.  In particular, look for any
     * cells that have undeflowed to 0 possible values.  This can
     * happen if a guess is wrong in recursive descent.
     */
    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        for (col=0; col < NUM_VERT_CELLS; col++) {
            cell = &board->x_y_board[row][col];
            if ((1 > cell->num_possible_values) ||
              (DIMENSION_SIZE < cell->num_possible_values)) {
                return (FALSE);
            }
        }
    }

    for (row=0; row < NUM_VERT_CELLS; row++) {
        /* Create a pointer array for the cells in each row dimension */
        get_relevant_cells(board, row, 0, ROW, test_cells);
        rc = test_sanity_of_dimension_never_solved(test_cells);
        if (!rc) {
            return (FALSE);
        }
    }

    for (col=0; col < NUM_HORIZ_CELLS; col++) {
        /* Create a pointer array for the cells in each column dimension */
        get_relevant_cells(board, 0, col, COL, test_cells);
        rc = test_sanity_of_dimension_never_solved(test_cells);
        if (!rc) {
            return (FALSE);
        }
    }

    for (row=0; row < NUM_VERT_CELLS; row += NUM_VERT_CELLS_IN_SQUARE) {
        for (col=0; col < NUM_HORIZ_CELLS; col += NUM_HORIZ_CELLS_IN_SQUARE) {
            /* Create a pointer array for the cells in each square dimension */
            get_relevant_cells(board, row, col, SQUARE, test_cells);
            rc = test_sanity_of_dimension_never_solved(test_cells);
            if (!rc) {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}

/*
 * Compute the square number from the row and column.
 * Square number starts with zero, increasing from left to right, and
 * top to bottom.  Mapping from (square_row, square_col) to square number
 * in a standard game:
 * 
 * (0, 0) (0, 1) (0, 2)       0 1 2
 * (1, 0) (1, 1) (1, 2)  ==>  3 4 5
 * (2, 0) (2, 1) (2, 2)       6 7 8
 * 
 * cell_row:  Horizontal row of cell, zero indexed
 * cell_col:  Vertical column of cell, zero indexed
 */
uint4
get_square_num(uint4 cell_row, uint4 cell_col)
{
    uint4    square_row, square_col, square_num;

    square_row = cell_row / NUM_HORIZ_CELLS_IN_SQUARE;
    square_col = cell_col / NUM_VERT_CELLS_IN_SQUARE;
    square_num = square_row * NUM_HORIZ_SQUARES + square_col;

    return (square_num);
}

/*
 * Initialize the game
 */
board_t *
game_init(game_t *game)
{
    uint4    row, col;
    uint4    i = 0;

    game->board_head = malloc(sizeof(board_t));
    if (!game->board_head) {
        return (NULL);
    }
    game->board_curr = game->board_head;
    board_set_pointers(game->board_curr, game, NULL, NULL);

    game->filename = NULL;
    game->game_flags = 0;
    game->max_num_recursion_levels = DEFAULT_MAX_NUM_RECURSION_LEVELS;
    game->initial_start_index = 0;
    game->recurse_stats = NULL;

    /* Initialize the "map index to coordinates" lookup table */
    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        for (col=0; col < NUM_VERT_CELLS; col++) {
            game->coord[i].row = row;
            game->coord[i].col = col;
            i++;
        }
    }

    return (game->board_head);
}

/*
 * Free dynamically allocated memory associated with the game.
 */
void
game_cleanup(game_t *game)
{
    board_t   *board_tail;
    board_t   *board_tmp;

    if (game->filename) {
        free(game->filename);
    }
    if (game->recurse_stats) {
        free(game->recurse_stats);
    }

    /*
     * Free the board_t structs (one or more) that are linked from the
     * head of the game list.
     * Fixme:  Move this to a separate function that allows a partial
     *   list to be freed.
     */
    board_tail = game->board_head;

    while (NULL != board_tail->next) {
        board_tail = board_tail->next;
    }

    do {
        board_tmp = board_tail;
        board_tail = board_tail->prev;
        free(board_tmp);
    } while (NULL != board_tail);
}

/*
 * Initialize one cell of a game
 * 
 * row:  Horizontal row, zero indexed
 * col:  Vertical column, zero indexed
 */
void
cell_init(board_t *board, uint4 row, uint4 col)
{
    cell_t    *cell;
    uint4     i;

    cell = &board->x_y_board[row][col];

    for (i=0; i < DIMENSION_SIZE; i++) {
        cell->values[i] = TRUE;
    }
    cell->num_possible_values = DIMENSION_SIZE;
    cell->mark_flag           = FALSE;

    cell->row                 = row;
    cell->col                 = col;
    cell->square              = get_square_num(row, col);
}

/*
 * Allocate and zero a recurse_stats_t structure.
 */
recurse_stats_t *
alloc_init_game_stats(void)
{
    recurse_stats_t  *recurse_stats;

    recurse_stats = malloc(sizeof(recurse_stats_t));
    if (!recurse_stats) {
        return (NULL);
    }

    memset(recurse_stats, 0, sizeof(recurse_stats_t));

    return (recurse_stats);
}

/*
 * Initialize all pointers in a board_t structure.
 */
void
board_set_pointers(board_t *board, game_t *game, board_t *prev, board_t *next)
{
    board->game = game;
    board->prev = prev;
    board->next = next;
}

/*
 * Initialize all cells in a board.
 */
void
board_init(board_t *board)
{
    uint4    row, col;

    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        for (col=0; col < NUM_VERT_CELLS; col++) {
            cell_init(board, row, col);
        }
    }
}

/*
 * Find the first undetermined cell in the board, if any.
 * Return a pointer to the cell, if any, else NULL.
 */
cell_t *
board_find_first_undetermined_cell(board_t *board)
{
    cell_t   *cell;
    uint4    row, col;

    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        for (col=0; col < NUM_VERT_CELLS; col++) {
            cell = &board->x_y_board[row][col];
            if (cell->num_possible_values > 1) {
                return (cell);
            }
        }
    }
    return (NULL);
}

/*
 * Finds the first undetermined cell in the board, if any, which is not
 * before the position indicated by "index".
 *   state->state_index:
 *     - passed in by the caller
 *     - traditionally initialized to 0 by the caller before the first call
 *     - (unimplemented) initialized to some value 0 <= value <= TOT_NUM_CELLS
 *     -  by the caller before the first call.  Allows random seeding
 *        of the search start value
 *     - updated by this function before return
 *     - allows this function to track the state of where to start
 *        looking for the next undetermined cell
 *     - must not be modified by the caller
 *   state->state_count:
 *     - passed in by the caller
 *     - initialized to 0 by the caller before the first call
 *     - updated by this function before return
 *     - allows this function to track the state of how many cells
 *        have been examined.  Used to detect terminal condition.
 *     - must not be modified by the caller
 * Returns a pointer to the cell, if any, else NULL.
 */
cell_t *
board_find_next_undetermined_cell(game_t *game, board_t *board,
                                  bfnuc_state_t *state)
{
    cell_t   *cell;
    uint4    row, col;
    uint4    internal_index;
    uint4    internal_count;

    internal_count = state->state_count;
    if (TOT_NUM_CELLS <= internal_count) {
        // Fixme
        //printf("returning cell %p row %u col %u\n", NULL, 0, 0);

        return (NULL);
    }
    internal_index = state->state_index;

    do {
        row = game->coord[internal_index].row;
        col = game->coord[internal_index].col;
        cell = &board->x_y_board[row][col];

        if (cell->num_possible_values > 1) {
            state->state_index = internal_index + 1;
            if (internal_index >= TOT_NUM_CELLS) {
                state->state_index = 0;
            }
            state->state_count = internal_count + 1;

            // Fixme
            //printf("returning cell %p row %u col %u i %u cnt %u\n",
            //    cell, row, col, state->state_index, state->state_count);

            return (cell);
        }
        internal_index++;
        /* Fixed wrap bug */
        if (internal_index >= TOT_NUM_CELLS) {
            internal_index = 0;
        }
        internal_count++;
    } while (internal_count <= TOT_NUM_CELLS);

    state->state_index = internal_index;
    state->state_count = internal_count;

    // Fixme
    //printf("returning cell %p row %u col %u\n", NULL, row, col);

    return (NULL);
}

/*
 * Seed the random number generator, which is optionally used in
 * selecting a starting cell index for searching a board.
 * Works for FreeBSD.  Should work for Linux, but has not been tested.
 */
void
seed_random_number_generator(void)
{
    srandomdev();
}

/*
 * Return the starting cell index for searching a board.  The value
 * returned is a random index if requested; else a specified value, if
 * provided; else 0.
 * Works for FreeBSD.  Should work for Linux, but has not been tested.
 */
uint4
board_get_initial_cell_index(game_t *game)
{
    long     rand_raw;
    uint4    rand_val;

    if (RANDOM_START_INDEX & game->game_flags) {
        rand_raw = random();
        rand_val = rand_raw % TOT_NUM_CELLS;

        return (rand_val);
    } else {
        return (game->initial_start_index);
    }
}

/*
 * Copy all elements of board_dst to board_src.
 * 
 * Important:  Overwrites the pointers in the destination board_t struct.
 */
void
board_copy(board_t *board_dst, board_t *board_src)
{
    memcpy(board_dst, board_src, sizeof(board_t));
}

/*
 * Copy all elements of board_dst to board_src.
 * 
 * Preserves the pointers in the destination board_t struct.
 */
void
board_copy_preserve_pointers(board_t *board_dst, board_t *board_src)
{
    game_t   *game_save = board_dst->game;
    board_t  *prev_save = board_dst->prev;
    board_t  *next_save = board_dst->next;

    board_copy(board_dst, board_src);

    board_set_pointers(board_dst, game_save, prev_save, next_save);
}

/*
 * Print the cell coordinates for all cells in a board.
 * For code development debug only.
 */
void
print_cell_coordinates(board_t *board)
{
    cell_t   *cell;
    uint4    row, col;

    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        for (col=0; col < NUM_VERT_CELLS; col++) {
            cell = &board->x_y_board[row][col];
            printf("(%u, %u, %u) ", cell->row, cell->col, cell->square);
        }
        printf("\n");
    }
}

/*
 * Initialize the game
 */
game_t *
sudoku_init(void)
{
    game_t   *game;
    board_t  *rc;

    game = malloc(sizeof(game_t));
    if (!game) {
        return (NULL);
    }

    rc = game_init(game);
    if (!rc) {
        free(game);
        return (NULL);
    }
    printd_init(&(game->dprint), "./logs/sudoku_log");

    board_init(game->board_curr);

    //print_cell_coordinates(game->board_curr);

    return (game);
}

/*
 * Clean up the game
 */
void
sudoku_cleanup(game_t *game)
{
    printd_cleanup(&(game->dprint));
    game_cleanup(game);

    if (game) {
        free(game);
    }
}

/*
 * Copy all elements of cell_dst to cell_src.
 */
void
cell_copy(cell_t *cell_dst, cell_t *cell_src)
{
    memcpy(cell_dst, cell_src, sizeof(cell_t));
}

/*
 * Set the given value in a cell, and clear all others.
 */
void
set_cell_value(cell_t *cell, uint4 value)
{
    uint4     i;

    for (i=0; i < DIMENSION_SIZE; i++) {
        cell->values[i] = FALSE;
    }

    cell->values[value - 1]   = TRUE;
    cell->num_possible_values = 1;
}

/*
 * If set, clear the specified value in a cell and decrement the count
 */
bool
clear_cell_value(cell_t *cell, uint4 value)
{
    if (cell->values[value - 1]) {
        cell->values[value - 1]   = FALSE;
        cell->num_possible_values--;
        return (TRUE);
    }

    return (FALSE);
}

/*
 * Return TRUE if "value" is set in the cell, else return FALSE.
 */
bool
cell_value_set(cell_t *cell, uint4 value)
{
    if (cell->values[value - 1]) {
        return (TRUE);
    }

    return (FALSE);
}

/*
 * Get the numerically lowest value set in a cell
 */
uint4
get_lowest_cell_value(cell_t *cell)
{
    uint4     i;

    for (i=0; i < DIMENSION_SIZE; i++) {
        if (cell->values[i]) {
            return (i + 1);
        }
    }

    return (-100);
}

/*
 * Returns TRUE if all marked cells in the passed in dimension are in the
 * same row or column.  If TRUE, also returns the dimension (ROW or COL)
 * and the relevant zero-indexed row or column number.
 * Returns FALSE otherwise.  Returns FALSE if no cells are marked.
 */
bool
all_marked_cells_in_same_dimension(cell_t *test_cells[DIMENSION_SIZE],
                                   uint4 *row, uint4 *col,
                                   uint4 *dimension)
{
    uint4     first_row, first_col;
    uint4     i;
    cell_t    *cell;
    bool      success;

    success = FALSE;

    /* Find the first marked cell in the passed in dimension */
    for (i=0; i < DIMENSION_SIZE; i++) {
        cell = test_cells[i];
        if (cell_is_marked(cell)) {
            first_row = cell->row;
            first_col = cell->col;
            success = TRUE;
            break;
        }
    }

    /* Fail if no marked cell found */
    if (!success) {
        return (FALSE);
    }

    success = TRUE;

    /* Check if all marked cells are in the same row */
    for (i=0; i < DIMENSION_SIZE; i++) {
        cell = test_cells[i];
        if (cell_is_marked(cell)) {
            if (first_row != cell->row) {
                success = FALSE;
                break;
            }
        }
    }

    /* Success if all marked cells are in the same row */
    if (success) {
        *row = first_row;
        *col = 0;
        *dimension = ROW;

        return (TRUE);
    }

    success = TRUE;

    /* Check if all marked cells are in the same column */
    for (i=0; i < DIMENSION_SIZE; i++) {
        cell = test_cells[i];
        if (cell_is_marked(cell)) {
            if (first_col != cell->col) {
                success = FALSE;
                break;
            }
        }
    }

    /* Success if all marked cells are in the same column */
    if (success) {
        *row = 0;
        *col = first_col;
        *dimension = COL;

        return (TRUE);
    }

    return (FALSE);
}

/*
 * If "val" exists in any of the unmarked cells, clear it.
 * No frills version of clear_val_in_unmarked_cells_in_dimension().
 *
 * val:  zero-indexed value (0-8 for standard game)
 *
 * Returns number of cells in which val was cleared
 */
static uint4
clear_values_in_unmarked_cells(cell_t *test_cells[DIMENSION_SIZE], uint4 val)
{
    cell_t    *cell;
    uint4     i;
    uint4     num_changes = 0;

    for (i=0; i < DIMENSION_SIZE; i++) {
        cell = test_cells[i];
        if (!cell_is_marked(cell)) {
            /* If the value is set in the cell, clear it. */
            if (clear_cell_value(cell, val + 1)) {
                num_changes++;
            }
        }
    }

    return (num_changes);
}

/*
 * If "val" exists in any of the unmarked cells, clear it.
 *
 * val:  zero-indexed value (0-8 for standard game)
 *
 * Returns number of cells in which val was cleared
 */
static uint4
clear_val_in_unmarked_cells_in_dimension(cell_t *test_cells[DIMENSION_SIZE],
                                         uint4 val, uint4 row, uint4 col,
                                         uint4 dimension)
{
    cell_t    *cell;
    uint4     i;
    uint4     num_changes = 0;

    printd_dimension(D_DIM, test_cells);

    for (i=0; i < DIMENSION_SIZE; i++) {
        cell = test_cells[i];
        if (!cell_is_marked(cell)) {
            /* If the value is set in the cell, clear it. */
            if (clear_cell_value(cell, val + 1)) {
                num_changes++;
                printd(D_DIM, "  >>>>> clear_unmarked:  clearing %u ",
                    val + 1);
                printd(D_DIM, "in (%u, %u, %u) values:  |",
                    cell->row, cell->col, cell->square);
                printd_cell_cell(D_DIM, cell);
                    printd(D_DIM, "\n");
            }
        }
    }

    return (num_changes);
}

/*
 * Set the given value in a cell in a board, and clear all others.
 */
void
set_cell_value_board(board_t *board, uint4 row, uint4 col, uint4 value)
{
    cell_t    *cell;

    cell = &board->x_y_board[row][col];

    set_cell_value(cell, value);
}

/*
 * Returns TRUE if both cells contain the exact same values,
 * else FALSE.
 */
bool
cell_equals(cell_t *cell1, cell_t *cell2)
{
    uint4     i;

    for (i=0; i < DIMENSION_SIZE; i++) {
        if (cell1->values[i] != cell2->values[i]) {
            return (FALSE);
        }
    }

    return (TRUE);
}

/*
 * Returns TRUE if cell2 is a subtuple, proper or otherwise,
 * of cell1
 */
bool
cell_is_subtuple(cell_t *cell1, cell_t *cell2)
{
    uint4     i;

    for (i=0; i < DIMENSION_SIZE; i++) {
        if (!cell2->values[i]) {
            continue;
        }
        if (cell2->values[i] != cell1->values[i]) {
            return (FALSE);
        }
    }

    return (TRUE);
}

/*
 * Mark the cell for subsequent special handling
 */
void
mark_cell(cell_t *cell)
{
    cell->mark_flag = TRUE;
}

/*
 * Clear the cell's special handling flag
 */
void
unmark_cell(cell_t *cell)
{
    cell->mark_flag = FALSE;
}

/*
 * Clear the special handling flag for all cells in a dimension
 */
void
unmark_all_cells(cell_t *test_cells[DIMENSION_SIZE])
{
    uint4     cell_index;
    cell_t    *cell;

    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        unmark_cell(cell);
    }
}

/*
 * Mark all cells in a dimension containing the given possible value.
 * Returns the the number of cells containing that value.
 */
uint4
mark_all_cells_containing_value(cell_t *test_cells[DIMENSION_SIZE], uint4 val)
{
    uint4     cell_index;
    uint4     val_cnt = 0;
    cell_t    *cell;

    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        if (cell->values[val]) {
            mark_cell(cell);
            val_cnt++;
        }
    }

    return (val_cnt);
}

/*
 * Check if the cell's special handling flag is set
 */
bool
cell_is_marked(cell_t *cell)
{
    return (cell->mark_flag);
}

/*
 * Unconditionally clear a single cell array value.
 */
void
clear_array_value(bool array[DIMENSION_SIZE], uint4 value)
{
    array[value - 1] = FALSE;
}

/*
 * Print the usage string
 */
void
usage(int argc, char **argv)
{
    printf("\nusage: %s [-?hbIrRV] [-d <debug flags>] [-D <max depth>]\n"
        "           [-i <start index>] [-s <silent level>] -f <file name>\n",
        argv[0]);
    printf(
        "  -b:		try breadth-first search if needed\n"
        "  -d:		set debug flags\n"
        "  -D:		maximum depth for depth-first recursion\n"
        "  -f:		specify puzzle filename\n"
        "  -?h:		print help and exit\n"
        "  -i:		specify starting index for puzzle search\n"
        "  -I:		enable random starting index for puzzle search\n"
        "  -r:		try recursive descent if needed\n"
        "  -R:		use only recursive descent (unimplemented)\n"
        "  -s:		silent mode level\n"
        "  -V:		print version string and exit\n");
}

/*
 * Reads the input file and populates the initial board in the game.
 */
int
input_puzzle_file(game_t *game, FILE *input_fd)
{
    uint4    intermediate_board_format[NUM_VERT_CELLS][NUM_HORIZ_CELLS];
    uint4    row;
    uint4    col;
    int      rc;

    /*
     * Reads the input file into the intermediate format.  Values are
     * range checked; zero represents '.' from the input file.
     */
    rc = read_puzzle_file(input_fd, intermediate_board_format);
    if (rc) {
        return (rc);
    }

    /*
     * Populates the initial board in the game.
     */
    for (row=0; row < NUM_VERT_CELLS; row++) {
        for (col=0; col < NUM_HORIZ_CELLS; col++) {
            if (intermediate_board_format[row][col]) {
                set_cell_value_board(game->board_curr, row, col,
                  intermediate_board_format[row][col]);
            }
        }
    }

    return (0);
}

/*
 * Processes command line arguments.
 * Loads values from input file to game structure.
 */
int
input_values(int argc, char **argv, game_t *game)
{
    FILE     *input_fd;
    int      opt;
    char     *fname = "----";
    char     *filename = NULL;
    uint4    debug = 0;
    uint4    levels;
    char     *endptr;
    int      rc;

    /*
     * Note:  Can hack puzzle name into program for testing like the following:
     */
    //fname = "./puzzles/d6.txt";

    /*
     * Command line options handling
     */
    while ((opt = getopt(argc, argv, "?bd:D:f:hi:IrRs:V")) != EOF) {
        switch (opt) {
        case 'b':
            game->game_flags |= BREADTH_FIRST_1LR;
            break;
        case 'd':
            debug = strtoul(optarg, &endptr, 0);
            if (errno != ERANGE && *endptr == '\0') {
                game->dprint.flags = debug;
            } else {
                printf("Invalid debug flags 0x%x\n", debug);
                free(filename);
                return (-1);
            }
            break;
        case 'D':
            levels = strtoul(optarg, &endptr, 0);
            if (errno != ERANGE && *endptr == '\0') {
                game->max_num_recursion_levels = levels;
            } else {
                printf("Invalid recursion levels %d\n", levels);
                free(filename);
                return (-1);
            }
            break;
        case 'f':
            filename = strdup(optarg);
            break;
        case 'i':
            levels = strtoul(optarg, &endptr, 0);
            if (errno != ERANGE && *endptr == '\0') {
                game->initial_start_index = levels;
            } else {
                printf("Invalid start index %d\n", levels);
                free(filename);
                return (-1);
            }
            if (TOT_NUM_CELLS <= levels) {
                printf("Start index too large %d\n", levels);
                free(filename);
                return (-1);
            }
            game->initial_start_index = levels;
            break;
        case 'I':
            game->game_flags |= RANDOM_START_INDEX;
            seed_random_number_generator();
            break;
        case 'r':
            game->game_flags |= REC_DESCENT;
            break;
        case 'R':
            game->game_flags |= REC_DESCENT_ONLY;
            break;
        case 's':
            levels = strtoul(optarg, &endptr, 0);
            if (errno != ERANGE && *endptr == '\0') {
                game->dprint.silent_level = levels;
            } else {
                printf("Invalid silent level %d\n", levels);
                free(filename);
                return (-1);
            }
            break;
        case 'V':
            printf("Version:  %s\n", rcsid);
            free(filename);
            return (1);
            break;
        case 'h':
        case '?':
            usage(argc, argv);
            free(filename);
            return (1);
            break;
        }
    }
    if (!filename) {
        filename = strdup(fname);
    }

    printd(D_INPUT, "input filename:  %s\n", filename);

    input_fd = fopen(filename, "r");
    if (!input_fd) {
        printd(D_CRIT, "\n*** File \"%s\" does not exist!\n", filename);
        free(filename);
        return (-1);
    }

    game->filename = filename;

#ifdef REMOVED
    /*
     * Fixme:  Recursive descent not implemented at present.
     */
    if ((REC_DESCENT | REC_DESCENT_ONLY) & game->game_flags) {
        printf("\n### WARNING:  Recursive descent not implemented!\n\n");
    }
#endif

    /*
     * Reads the input file and populates the initial board in the game
     */
    rc = input_puzzle_file(game, input_fd);
    if (rc) {
        fclose(input_fd);
        return (rc);
    }

    fclose(input_fd);

    return(0);
}

/*
 * Get an array of pointers to all cells in the specified dimension.
 */
int
get_relevant_cells(board_t *board, uint4 input_row, uint4 input_col,
                   uint4 dimension, cell_t **return_array)
{
    uint4     row, col;
    uint4     i = 0;

    if (!return_array) {
        return (-1);
    }

    switch (dimension) {
    case ROW:
        for (col=0; col < DIMENSION_SIZE; col++) {
            return_array[i++] = &board->x_y_board[input_row][col];
        }
        break;
    case COL:
        for (row=0; row < DIMENSION_SIZE; row++) {
            return_array[i++] = &board->x_y_board[row][input_col];
        }
        break;
    case SQUARE:
        /* For now, square must be specified by (0, 0) corner */
        if ((input_row % NUM_VERT_CELLS_IN_SQUARE != 0) ||
                               (input_col % NUM_HORIZ_CELLS_IN_SQUARE != 0)) {
            printf("***** ERROR 2!  Bad origin for square (%u %u)!\n",
                input_row, input_col);
            exit(5);
        }
        for (row=input_row; row < input_row + NUM_VERT_CELLS_IN_SQUARE; row++) {
            for (col=input_col;
                          col < input_col + NUM_HORIZ_CELLS_IN_SQUARE; col++) {
                return_array[i++] = &board->x_y_board[row][col];
            }
        }
        break;
    default:
        printf("***** ERROR 1!  Unknown dimension %u!\n", dimension);
        return (-1);
        break;
    }

    return (0);
}

/*
 * Set all the values in a boolean array to a single value.
 * Typically each boolean represents a possible number in a cell,
 * with TRUE meaning "this number is possible" in the cell.
 */
void
init_test_values(bool test_val[DIMENSION_SIZE], bool init_val)
{
    uint4     i;

    for (i=0; i < DIMENSION_SIZE; i++) {
        test_val[i] = init_val;
    }
}

/*
 * Walk through a dimension array of cells.  For each determined cell
 * (cell containing exactly one possible value), remove that value from
 * the test_val array.
 *
 * cells:  array of pointers to the cells of a dimension
 * test_val:  array of bool, presumably all values TRUE
 */
void clear_determined_values(cell_t *const cells[DIMENSION_SIZE],
                             bool test_val[DIMENSION_SIZE])
{
    cell_t    *cell;
    uint4     i, value;

    for (i=0; i < DIMENSION_SIZE; i++) {
        cell = cells[i];
        if (cell->num_possible_values == 1) {
            value = get_lowest_cell_value(cell);
            clear_array_value(test_val, value);
        }
    }
}

/*
 * For each undetermined cell in a dimension, find out which values
 * the cell cannot contain and eliminate them.
 *
 * In the given dimension, some cells may have known (determined)
 * values; those have already been eliminated from the
 * test_val array.  Remove those determined values from any
 * undetermined cells.
 *
 * test_cells:  array of pointers to the cells of a dimension
 */
uint4
cannot_be_analysis(cell_t *test_cells[DIMENSION_SIZE], uint4 dimension)
{
    /* Represents each cell (1-9 in standard 9x9 game) */
    bool      test_val[DIMENSION_SIZE];
    cell_t    *cell;
    uint4     cell_index;
    uint4     i;
    uint4     num_changes = 0;

    /*
     * Initially, assume all values (1-9 for standard Sudoku game) are
     * possible in this dimension.  This function marks all values TRUE.
     */
    init_test_values(test_val, TRUE);

    /*
     * Create a test array of possible values for undetermined cells
     * in this dimension.  Every determined value (single value in a cell)
     * is cleared (set FALSE) in the test array.
     */
    clear_determined_values(test_cells, test_val);

    /*
     * For each possible cell in this dimension
     */
    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        if (cell->num_possible_values == 1) {
            continue;
        }
        /*
         * For each possible value (1-9 in standard 9x9 game),
         * clear the value in this cell if already determined
         * elsewhere in this dimension.
         */
        for (i=0; i < DIMENSION_SIZE; i++) {
            if (!test_val[i]) {
                if (cell->values[i]) {
                    printd(D_CANTBE, ">>>>> Cannot be (%u, %u, %u):  |", 
                        cell->row, cell->col, cell->square);
                    printd_cell_cell(D_CANTBE, cell);

                    cell->values[i] = FALSE;
                    cell->num_possible_values--;
                    num_changes++;

                    printd(D_CANTBE, " ==> |");
                    printd_cell_cell(D_CANTBE, cell);
                    printd(D_CANTBE, "\n");
                }
            }
        }
    }

    return (num_changes);
}

/*
 * If an undetermined cell contains a value that is unique in
 * this dimension, make it a determined cell set to that value.
 *
 * test_cells:  array of pointers to the cells of a dimension
 */
uint4
must_be_analysis(cell_t *test_cells[DIMENSION_SIZE])
{
    /* Represents each cell (1-9 in standard 9x9 game) */
    bool      test_val[DIMENSION_SIZE];
    cell_t    *cell, *cell_marker;
    uint4     cell_index;
    uint4     i, value_marker = 0;
    uint4     num_cells_with_value;
    uint4     num_changes = 0;

    /*
     * Initially, assume all values (1-9 for standard Sudoku game) are
     * possible in this dimension.  This function marks all values TRUE.
     */
    init_test_values(test_val, TRUE);

    /*
     * Create a test array of possible values for undetermined cells
     * in this dimension.  Every determined value (single value in a cell)
     * in test_cells is cleared (set FALSE) in the test array.
     */
    clear_determined_values(test_cells, test_val);

    /*
     * For each possible value (1-9 in standard 9x9 game)
     */
    for (i=0; i < DIMENSION_SIZE; i++) {
        if (!test_val[i]) {
            continue;
        }
        num_cells_with_value = 0;
        cell_marker = NULL;

        /*
         * For each undetermined cell in the dimension, attempt to
         * find exactly one cell containing the value of interest.
         */
        for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
            cell = test_cells[cell_index];
            if (cell->num_possible_values == 1) {
                continue;
            }
            if (cell->values[i]) {
                cell_marker = cell;
                value_marker = i;
                num_cells_with_value++;
            }
        }

        /*
         * If exactly one undetermined cell contains the value of interest,
         * the cell must represent that value.  Clear all other values in
         * that cell.  The cell becomes a determined cell with that value.
         */
        if (num_cells_with_value == 1) {
            printd(D_MUSTBE, ">>>>> Must be (%u, %u, %u):  |", 
                cell_marker->row, cell_marker->col, cell_marker->square);
            printd_cell_cell(D_MUSTBE, cell_marker);

            num_changes += (cell_marker->num_possible_values - 1);
            set_cell_value(cell_marker, value_marker + 1);

            printd(D_MUSTBE, " ==> |");
            printd_cell_cell(D_MUSTBE, cell_marker);
            printd(D_MUSTBE, "\n");
        }
    }

    return (num_changes);
}

/*
 * Clear every value in cell that appears in match_cell.
 */
uint4
cell_clear_match_cell_values(cell_t *cell, cell_t *match_cell, uint4 flags)
{
    uint4     i;
    uint4     num_changes = 0;

    for (i=0; i < DIMENSION_SIZE; i++) {
        if (cell->values[i] && match_cell->values[i]) {
            if (clear_cell_value(cell, i + 1)) {
                num_changes++;
                printd(flags, "  >>>>> clear_match:  clearing %u\n", i + 1);
            }
        }
    }

    return (num_changes);
}

/*
 * For all unmarked cells in a dimension, clear every value that
 * appears in match_cell.
 */
uint4
process_tuple_match(cell_t *test_cells[DIMENSION_SIZE], cell_t *match_cell,
                    uint4 flags)
{
    cell_t    *cell;
    uint4     cell_index;
    uint4     num_changes = 0;

    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        if (cell_is_marked(cell)) {
            continue;
        }
        num_changes += cell_clear_match_cell_values(cell, match_cell, flags);
    }

    return (num_changes);
}

/*
 * Within a dimension, if two cells containing a matching pair
 * exist, then both values in the pairs are "spoken for."  Therefore,
 * those two values cannot exist in any other cell in that dimension
 * and may be cleared.
 *
 * Example:
 *   |   6  |  34  |   9  | 2348 |   7  |  34  |  235 |  123 | 1358 |
 * becomes:
 *   |   6  |  34  |   9  | 2  8 |   7  |  34  |  2 5 |  12  | 1 58 |
 *
 * In the example, the second and sixth cells contain "34".  This means
 * both cells must contain a 3 or a 4.  Ultimately, one cell will
 * contain a 3 and the other a 4.  Since both 3 and 4 are unquestionably
 * spoken for, the values 3 and 4 can be eliminated from all other cells
 * in this dimension.
 *
 * Fixme -- extend this to higher tuples (e.g. 3 matching triplets)
 */
uint4
pair_analysis(cell_t *test_cells[DIMENSION_SIZE], uint4 dimension)
{
    cell_t    *cell, *cell2;
    uint4     cell_index;
    uint4     i;
    uint4     num_changes;
    uint4     tot_num_changes = 0;

    /* For each cell in this dimension */
    /* Fixme:  should be DIMENSION_SIZE - 1? */
    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        if (cell->num_possible_values != 2) {
            continue;
        }
        /* For all cells ahead of the test cell */
        for (i=cell_index+1; i < DIMENSION_SIZE; i++) {
            if (cell_equals(cell, test_cells[i])) {
                printd(D_PAIR, ">>>>> Mod tup (%u, %u, %u) [%u] match:  |",
                    cell->row, cell->col, cell->square, dimension);

                printd_cell_cell(D_PAIR, cell);

                cell2 = test_cells[i];
                mark_cell(cell);
                mark_cell(cell2);

                /*
                 * In the unmarked cells of this dimension, clear any
                 * value matching the pair.  This means clearing one,
                 * the other, or both values of the pair if they appear
                 * in any other cells.
                 */
                num_changes = process_tuple_match(test_cells, cell, D_PAIR);

                printd(D_PAIR, " %u changes\n", num_changes);

                unmark_cell(cell);
                unmark_cell(cell2);
                tot_num_changes += num_changes;
            }
        }
    }

    return (tot_num_changes);
}

/*
 * Modified tuple analysis
 *
 * Within a dimension, if one cell containing three values exists, and
 * two other cells exist that are subtuples of the first cell, then
 * both values in the pairs are "spoken for."  Therefore, those three
 * values cannot exist in any other cell in that dimension and may be
 * cleared.
 *
 * this does subtuple analysis 1:  236 23 36
 * this does subtuple analysis 2:  236 236 36
 * should work for  (???):         236 236 236
 * also does other variants:
 *   shown to work for:            789 789 8
 *
 * Example:
 *   |  236 | 23678|   5  | 13678|   9  |  36  |   4  |  23  |  16  |
 * becomes:
 *   |  236 |  78  |   5  |  178 |   9  |  36  |   4  |  23  |   1  |
 *
 * Example:
 *   |   6  |  34  |   9  | 2348 |   7  |  34  |  235 |  123 | 1358 |
 * becomes:
 *   |   6  |  34  |   9  | 2  8 |   7  |  34  |  2 5 |  12  | 1 58 |
 *
 * Example (from l4-6.txt):
 *   | 12 7 | 1 7  | 2 7  |  5   | 4    |    9 | 13 7 |   8  |  6   |
 * becomes:
 *   | 12 7 | 1 7  | 2 7  |  5   | 4    |    9 |  3   |   8  |  6   |
 *
 * Example (from l4-6.txt):
 *   | 12 7 | 1 7  | 2 7  |12578 | 3    |   6  |    9 | 1478 |24578 |
 * becomes:
 *   | 12 7 | 1 7  | 2 7  |  5 8 | 3    |   6  |    9 |  4 8 | 45 8 |
 *
 * Example (from l4-6.txt):
 *   |  3   | 1 57 | 1 57 | 1  7 | 2    |  4   |    9 |  6   | 1 58 |
 * becomes:
 *   |  3   | 1 57 | 1 57 | 1  7 | 2    |  4   |    9 |  6   |    8 |
 *
 * Example (from l4-4.txt)(unexpected, but works correctly):
 *   |  3   |  689 |25689 | 1    |   6  |24567 | 4579 |  689 |456789|
 * becomes:
 *   |  3   |  689 |25    | 1    |   6  |245 7 | 457  |  689 |45 7  |
 *
 */
uint4
modified_tuple_analysis(cell_t *test_cells[DIMENSION_SIZE], uint4 dimension)
{
    uint4     cell_index;
    cell_t    *cell, *cell2;
    uint4     subtuple_count;
    uint4     i;
    uint4     num_changes;
    uint4     tot_num_changes = 0;

    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        subtuple_count = 0;
        cell = test_cells[cell_index];
        if (cell->num_possible_values != 3) {
            continue;
        }
        mark_cell(cell);
        //printf(">>>>> mod_tuple:  Marking cell!  >>>");
        //print_cell_cell_vals(cell);
        //printf("<<<\n");
        for (i=0; i < DIMENSION_SIZE; i++) {
            cell2 = test_cells[i];
            if (cell_is_marked(cell2)) {
                continue;
            }
            if (cell_is_subtuple(cell, cell2)) {
                subtuple_count++;
                mark_cell(cell2);
            }
        }
        if (subtuple_count >= 2) {
            if (subtuple_count > 2) {
                printd(D_MTUP, "*** %s:  %u Subtuples > 2!\n",
                     __func__, subtuple_count);
            }
            printd(D_MTUP, ">>>>> Mod tup (%u, %u, %u) [%u] match:  |", 
                cell->row, cell->col, cell->square, dimension);
            printd_cell_cell(D_MTUP, cell);
            printd(D_MTUP, " <<<\n");

            printd_dimension(D_MTUP, test_cells);

            num_changes = process_tuple_match(test_cells, cell, D_MTUP);
            tot_num_changes += num_changes;

            printd_dimension(D_MTUP, test_cells);

            printd(D_MTUP, ">>>>> Mod tup:  %u changes\n", num_changes);
        }
        unmark_all_cells(test_cells);
    }
    unmark_cell(cell);
// Fixme
    unmark_all_cells(test_cells);

    return (tot_num_changes);
}

/*
 * All serpentines (for the time being) involve exactly three cells.
 * The sums of the bitmasks for those three cells will have exactly
 *   three values set. 
 * Those three values will all be two or higher.
 * If all of thise three values are two, then we have a "perfect serpentine."
 *   Example:  AB BC AC     12 23 13     vals:  222 000 000
 *   Example:  AB BC ABC    12 23 123    vals:  232 000 000
 *   Example:  AB ABC ABC   12 123 123   vals:  332 000 000
 *   Example:  ABC ABC ABC  123 123 123  vals:  333 000 000
 *
 * Note:  d6.txt contains a perfect serpentine:  34 23 24
 * Note:  d6.txt contains a perfect serpentine:  56 25 26, which leads to
 *   four values being cleared:
 *  |   56  |  4    | 3     | 125 9 | 125 9 | 2  5  |    7  |    8  | 2  6  |
 * becomes:
 *  |   56  |  4    | 3     | 1   9 | 1   9 | 2  5  |    7  |    8  | 2  6  |
 *
 * Note:  m3.txt contains a serpentine:  269 29 26, which leads to
 *   four values being cleared:
 *  | 2 6 9 | 3     | 2   9 |    7  | 126 8 |   5   | 2   6 | 126 8 |  4    |
 * becomes:
 *  | 2 6 9 | 3     | 2   9 |    7  | 1   8 |   5   | 2   6 | 1   8 |  4    |
 *
 * Note:  m4.txt contains a serpentine:  278 27 78, which leads to
 *   five values being cleared:
 *  | 24789 | 2 78  | 1     |   5   | 2  7  |  3    |    6  |    78 | 4 789 |
 * becomes:
 *  |  4  9 | 2 78  | 1     |   5   | 2  7  |  3    |    6  |    78 | 4   9 |
 *
 * Note:  m9.txt contains a serpentine:  46 146 146, which leads to
 *   four values being cleared:
 *  | 34 67 |  4 6  | 34 67 |     9 | 1 4 6 |   5   | 2     | 1 4 6 |    8  |
 * becomes:
 *  | 3   7 |  4 6  | 3   7 |     9 | 1 4 6 |   5   | 2     | 1 4 6 |    8  |
 *
 * Note:  m18.txt contains a serpentine:  568 568 568, which leads to
 *   four values being cleared:
 *  |  4    | 1     | 25 78 |     9 |  3    |  56 8 |  56 8 |  56 8 | 2 678 |
 *  |  4    | 1     | 2  7  |     9 |  3    |  56 8 |  56 8 |  56 8 | 2  7  |
 *
 * Note that serpentine analysis appears to have substantial overlap with
 *   modified tuple analysis.
 *
 * Fixme:  Look into extending this to four cells/values.
 */
uint4
serpentine_analysis(cell_t *test_cells[DIMENSION_SIZE])
{
    uint4     cell_index;
    cell_t    *cell;
    uint4     vals[DIMENSION_SIZE];
    uint4     val_index;
    /* number of cells representing 2 or 3 values */
    uint4     cell_count_23 = 0;
    /* number of values in marked cells occuring 2 or 3 times */
    uint4     val_count_23 = 0;
    /* number of values in marked cells occuring exactly 2 times (unneeded) */
    uint4     val_count_2 = 0;
    /* number of values in in marked cells occuring 0 times */
    uint4     val_count_0 = 0;
    uint4     num_changes = 0;

    /*
     * Record the number of cells representing 2 or 3 values.
     * Mark each of those cells.
     */
    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        if (cell->num_possible_values < 2 || cell->num_possible_values > 3) {
            continue;
        }
        cell_count_23++;
        mark_cell(cell);
    }

#ifdef REMOVED
// Fixme
// Fixme:  For code development debug only.
// Fixme
    if (cell_count_23 > 3) {
        //printd(D_SERP, "*** %s:  cell_count_23 %u\n",
        printf("*** %s:  cell_count_23 %u\n",
            __func__, cell_count_23);
        //printd_dimension(D_SERP, test_cells);
        printd_dimension(D_CRIT, test_cells);
    }
#endif
#ifdef REMOVED
    if (cell_count_23 == 3) {
        printd(D_SERP, "*** %s:  cell_count_23 %u\n",
            __func__, cell_count_23);
        printd_dimension(D_SERP, test_cells);
    }
#endif

    /*
     * This serpentine analysis requires exactly 3 cells, each containing
     * 2 or 3 values.
     * Fixme:  This is unduly restrictive.  It makes the algorithm simpler,
     * though.  If > 3, could iterate through subsets of 3.
     */
    if (3 != cell_count_23) {
        unmark_all_cells(test_cells);
        return(0);
    }

    for (val_index=0; val_index < DIMENSION_SIZE; val_index++) {
        vals[val_index] = 0;
    }

    /*
     * For each of the marked cells, sum the occurrences of each value
     * that they represent.
     */
    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        if (cell_is_marked(cell)) {
            for (val_index=0; val_index < DIMENSION_SIZE; val_index++) {
                if (cell->values[val_index]) {
                    vals[val_index]++;
                }
            }
        }
    }

    /*
     * This serpentine analysis requires exactly 3 values to be
     * represented, and each must be represented 2 or 3 times.
     * All remaining values must not be represented at all.
     */
    for (val_index=0; val_index < DIMENSION_SIZE; val_index++) {
        if (0 == vals[val_index]) {
            val_count_0++;
        }
        if (2 == vals[val_index]) {
            val_count_2++;
        }
        if ((2 == vals[val_index]) || (3 == vals[val_index])) {
            val_count_23++;
        }
    }

#ifdef REMOVED
    printd(D_SERP, "    vc0 %u vc2 %u vc3 %u\n",
        val_count_0, val_count_2, val_count_23);
#endif

    if (6 != val_count_0) {
        unmark_all_cells(test_cells);

        return(0);
    }
    if ((val_count_23 < 2) || (val_count_23 > 3)) {
        unmark_all_cells(test_cells);

        return(0);
    }

    // Fixme
    if (cell_count_23 == 3) {
        printd(D_SERP, "*** %s:  cell_count_23 %u\n",
            __func__, cell_count_23);
        printd_dimension(D_SERP, test_cells);
    }

    /*
     * We can clear the values represented by the cells in the serpentine
     * from the balance of the cells in the dimension.
     */
    for (val_index=0; val_index < DIMENSION_SIZE; val_index++) {
        if (vals[val_index]) {
            num_changes += clear_values_in_unmarked_cells(test_cells,
                val_index);
        }
    }

    if (num_changes) {
        printd(D_SERP, "*** %s:  cell_count_23 %u\n",
            __func__, cell_count_23);
        printd_dimension(D_SERP, test_cells);
        printd(D_SERP, "    Cleared %d values! vc0 %u vc2 %u vc3 %u\n",
            num_changes, val_count_0, val_count_2, val_count_23);
    }

    unmark_all_cells(test_cells);

    return (num_changes);
}

/*
 * Test the marked cells in the cell_ptrs[] array, and determine if they
 * constitute a serpentine.  If so, clear the values those cells represent
 * from the balance of the cells in the dimension.
 *
 * test_cells:    Array of all cell pointers in the dimension.
 * cell_ptrs:     Array of cell pointers of interest from the dimension.
 * cell_count:    Number of valid cells in cell_ptrs array.
 * choose_count:  Number of cells to choose from cell_ptrs array.
 *                - this number of cells must be marked
 */
uint4
new_serpentine_helper(cell_t *test_cells[DIMENSION_SIZE],
                      cell_t *cell_ptrs[DIMENSION_SIZE], uint4 cell_count,
                      uint4 choose_count)
{
    uint4     cell_index;
    uint4     val_index;
    cell_t    *cell;
    uint4     vals[DIMENSION_SIZE] = { 0 };
    uint4     num_changes = 0;
    /* number of values in marked cells occuring 2, 3 or 4 times */
    uint4     val_count_234 = 0;
    /* number of values in marked cells occuring 2 or 3 times */
    uint4     val_count_23 = 0;
    /* number of values in marked cells occuring 0 times */
    uint4     val_count_0 = 0;

    /*
     * For each cell in the cell_ptrs[] array, sum the occurrences of
     * all values they represent.
     */
    for (cell_index=0; cell_index < cell_count; cell_index++) {
        cell = cell_ptrs[cell_index];
        if (cell_is_marked(cell)) {
            for (val_index=0; val_index < DIMENSION_SIZE; val_index++) {
                if (cell->values[val_index]) {
                    vals[val_index]++;
                }
            }
        }
    }

    /*
     * This serpentine analysis requires exactly "choose_count" values
     * (currently 3 or 4) to be represented.  Each must be represented:
     *   choose_count == 3:   2 or 3 times
     *   choose_count == 4:   2, 3 or 4 times
     * All remaining values must not be represented at all.
     */
    for (val_index=0; val_index < DIMENSION_SIZE; val_index++) {
        /* number of values not represented */
        if (0 == vals[val_index]) {
            val_count_0++;
        }
        if ((2 == vals[val_index]) || (3 == vals[val_index])) {
            val_count_23++;
        }
        if ((1 < vals[val_index]) && (5 > vals[val_index])) {
            val_count_234++;
        }
    }

#ifdef REMOVED
    printd(D_SERP, "    vc0 %u vc23 %u vc234 %u\n",
        val_count_0, val_count_23, val_count_234);
#endif

    /*
     * Number of values not represented.
     * - choose_count 3:  val_count_0 must be exactly 6 for 9 cell dimension
     * - choose_count 4:  val_count_0 must be exactly 5 for 9 cell dimension
     */
    if ((DIMENSION_SIZE - choose_count) != val_count_0) {
        return(0);
    }

    switch (choose_count) {
    case 3:
        if ((val_count_23 < 2) || (val_count_23 > 3)) {
            return(0);
        }
        break;
    case 4:
        /*
         * Four values must be represented 2, 3, or 4 times each.
         * Note:  If we have four values represented 2 times each,
         * we have one of two scenarios:
         * - AB AB CD CD  pointless to pursue, but harmless
         *                - pairs that are mutually exclusive
         * - AB BC CD DA  must clear these values from other cells.
         */
        if ((val_count_234 < 2) || (val_count_234 > 4)) {
            return(0);
        }
        break;
    default:
        break;
    }

    printd(D_SERP, "*** %s:  cell_count %u choose_count %u\n",
        __func__, cell_count, choose_count);
    printd_dimension(D_SERP, test_cells);

    /*
     * We can clear the values represented by the cells in the serpentine
     * from the balance of the cells (unmarked cells) in the dimension.
     */
    for (val_index=0; val_index < DIMENSION_SIZE; val_index++) {
        if (vals[val_index]) {
            num_changes += clear_values_in_unmarked_cells(test_cells,
                val_index);
        }
    }

    if (num_changes) {
        printd(D_SERP, "*** %s:  cell_count %u choose_count %u\n",
            __func__, cell_count, choose_count);
        printd_dimension(D_SERP, test_cells);
        printd(D_SERP, "    Cleared %d values! vc0 %u vc23 %u vc234 %u\n",
            num_changes, val_count_0, val_count_23, val_count_234);
    }

    return (num_changes);
}

#define MASK_3_3_NUM_BITS		3
#define MASK_3_3_NUM_BITMASKS		1

/* 3 choose 3 bitmask:  111 */
static uint4 mask_3_3[MASK_3_3_NUM_BITMASKS] = 
                       { 0x7 };

#define MASK_4_3_NUM_BITS		4
#define MASK_4_3_NUM_BITMASKS		4

/* 4 choose 3 bitmask:  1110 1101 1011 0111 */
static uint4 mask_4_3[MASK_4_3_NUM_BITMASKS] =
                       { 0xe, 0xd, 0xb, 0x7 };

#define MASK_5_3_NUM_BITS		5
#define MASK_5_3_NUM_BITMASKS		10

/*
 * 5 choose 3 bitmask:  11100 11010 10110 01110
 *                      11001 10101 01101
 *                      10011 01011
 *                      00111
 */
static uint4 mask_5_3[MASK_5_3_NUM_BITMASKS] =
                       { 0x1c, 0x1a, 0x16, 0x0e,
                         0x19, 0x15, 0x0d,
                         0x13, 0x0b,
                         0x07 };

#define MASK_4_4_NUM_BITS		4
#define MASK_4_4_NUM_BITMASKS		1

/* 4 choose 4 bitmask:  1111 */
static uint4 mask_4_4[MASK_4_4_NUM_BITMASKS] = 
                       { 0xf };

#define MASK_5_4_NUM_BITS		5
#define MASK_5_4_NUM_BITMASKS		5

/* 4 choose 4 bitmask:  01111 10111 11011 11101 11110 */
static uint4 mask_5_4[MASK_5_4_NUM_BITMASKS] = 
                       { 0x0f, 0x17, 0x1b, 0x1d, 0x1e };

/*
 * Performs serpentine analysis on (M choose N) subsets of the selected cells
 * in a dimension.
 *
 * test_cells:  an array of pointers to all of the cells in the
 *       dimension of interest
 *       - all DIMENSION_SIZE number of cells in the dimension
 * cell_ptrs:  an array of pointers to the "cell_count" number of
 *       cells that the caller wants us to test for serpentines
 *       - this is a subset of the cells in the dimension
 * cell_count:  The M of (M choose N)
 *       - the number of cells in the cell_ptrs array
 * choose_count:  The N of (M choose N)
 *       - the size of the sets of cells to test
 *       - less than or equal to cell_count
 */
uint4
new_serpentine_combinations_analysis(cell_t *test_cells[DIMENSION_SIZE],
                                     cell_t *cell_ptrs[DIMENSION_SIZE],
                                     uint4 cell_count, uint4 choose_count)
{
    uint4     num_changes;
    uint4     num_bits;
    uint4     num_bitmasks;
    uint4     bitmask;
    uint4     nbm;
    uint4     nb;
    uint4     *bitmask_arr;

    switch (choose_count) {
    case 3:
        /* (3 choose 3), (4 choose 3), (5 choose 3) */
        switch (cell_count) {
        case 3:
            bitmask_arr  = mask_3_3;
            num_bits     = MASK_3_3_NUM_BITS;
            num_bitmasks = MASK_3_3_NUM_BITMASKS;
            break;
        case 4:
            bitmask_arr  = mask_4_3;
            num_bits     = MASK_4_3_NUM_BITS;
            num_bitmasks = MASK_4_3_NUM_BITMASKS;
            break;
        case 5:
            bitmask_arr  = mask_5_3;
            num_bits     = MASK_5_3_NUM_BITS;
            num_bitmasks = MASK_5_3_NUM_BITMASKS;
            break;
        default:
            printd(D_SERP, "<<<<<< cell count error choose %d count %d\n",
                choose_count, cell_count);

            return (0);
        }
        break;
    case 4:
        /* (4 choose 4), (5 choose 4) */
        switch (cell_count) {
        case 4:
            bitmask_arr  = mask_4_4;
            num_bits     = MASK_4_4_NUM_BITS;
            num_bitmasks = MASK_4_4_NUM_BITMASKS;
            break;
        case 5:
            bitmask_arr  = mask_5_4;
            num_bits     = MASK_5_4_NUM_BITS;
            num_bitmasks = MASK_5_4_NUM_BITMASKS;
            break;
        default:
            printd(D_SERP, "<<<<<< cell count error choose %d count %d\n",
                choose_count, cell_count);

            return (0);
        }
        break;
    default:
        printf("<<<<<< choose count error choose %d count %d\n",
            choose_count, cell_count);

        return (0);
    }

    /*
     * The caller passed in "choose_count and "cell_count".
     * Here we implement the (M choose N) (actually "cell_count" choose
     * "choose_count") subsets of cells from the dimension.  We mark N
     * cells in the M cell subset from the dimension, and call
     * new_serpentine_helper().
     * 
     * The helper function determines if the N
     * cells constitute a serpentine; if so, it can clear all values
     * represented by the N cells from the balance of the cells in the
     * dimension.
     * 
     * nbm:  number of bit masks
     *       - also the number of permutations
     * nb:   number of bits
     *       - also the number of cells to choose
     *       - also "choose_count"
     *       - also "N" in the comment above
     * cell_ptrs:  an array of pointers to the M (choose_count) number of
     *       cells that the caller wants us to test for serpentines
     *       - this is a subset of the dimension
     * test_cells:  an array of pointers to all of the cells in the
     *       dimension of interest
     *       - all DIMENSION_SIZE number of cells in the dimension
     */
    for (nbm=0; nbm < num_bitmasks; nbm++) {
        bitmask = bitmask_arr[nbm];
        for (nb=0; nb < num_bits; nb++) {
            if (bitmask & 0x1) {
                mark_cell(cell_ptrs[nb]);
            }
            bitmask = bitmask >> 1;
        }
        num_changes = new_serpentine_helper(test_cells, cell_ptrs,
          cell_count, choose_count);

        // Fixme:  We are responsible for unmarking cells.
        //unmark_all_cells_array(array, num_cells);
        unmark_all_cells(test_cells);

        /*
         * If the helper function cleared any values, we stop working on this
         * dimension.  We don't want to continue once things have changed.
         * The caller can call us again if they want to make another attempt.
         */
        if (num_changes) {
            printd(D_SERP, "<<<<<< nbm %d n_bm %d mask 0x%x\n",
                nbm, num_bitmasks, bitmask_arr[nbm]);

            return (num_changes);
        }
    }

    return (0);
}

/*
 * Choose a set of the form (M choose N) from the cells in the provided
 * dimension.  The set will be all of the cells representing (2 through N)
 * different values, where N is "choose_count."  We take no further
 * action if the set does not contain at least two cells.
 */
uint4
new_serpentine_m_choose_n_analysis(cell_t *test_cells[DIMENSION_SIZE],
                                   uint4 choose_count)
{
    uint4     cell_index;
    uint4     num_changes;
    /* # of cells representing (2 through "choose_count") values, inclusive */
    uint4     cell_count = 0;
    cell_t    *cell_ptrs[DIMENSION_SIZE] = { 0 };
    cell_t    *cell;

    /*
     * Record the number of cells representing 2 through "choose_count"
     * values, inclusive.  Store pointers to each of those cells in
     * cell_ptrs[].
     */
    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        if (cell->num_possible_values < 2 ||
          cell->num_possible_values > choose_count) {
            continue;
        }
        cell_ptrs[cell_count] = cell;
        cell_count++;
    }

    /*
     * We are testing all ("cell_count" choose "choose_count") permutations
     * of the cells discovered above.  This makes no sense for
     * (cell_count < choose_count).
     */
    if (choose_count > cell_count) {
        return (0);
    }

    /*
     * Perform serpentine analysis on all the (M choose N) [actually
     * ("cell_count" choose "chose_count")] subsets of the
     * cells pointed to by the cell_ptrs array.
     */
    num_changes = new_serpentine_combinations_analysis(test_cells, cell_ptrs,
      cell_count, choose_count);

    return (num_changes);
}

/*
 * All serpentines (at this time) involve exactly three cells, or
 * exactly four cells.
 * Those serpentines can be chosen from a larger number of cells, if a
 * larger number of cells is found in a dimension that meets the criteria.
 * 
 * Currently, (5 choose 4), (4 choose 4), (5 choose 3), (4 choose 3) and
 * (3 choose 3) are implemented.  This means that all subsets are tested
 * in turn.  Using (4 choose 3) as an example, all four unordered sets of
 * three will be chosen from the eligible set of four, then tested for
 * serpentine properties.
 * 
 * A serpentine completely covers a set of numbers; this means that all the
 * numbers represented by the set of cells in the serpentine are "spoken
 * for," and can therefore be cleared in all other cells in the dimension.
 * For an (M choose 3) serpentine, three numbers will be spoken for.
 * For an (M choose 4) serpentine, four numbers will be spoken for.
 * 
 * The sums of the bitmasks for the three cells in an (M choose 3) serpentine
 * will have exactly three values set.  Those three values will each have
 * a value of two or three.
 * If all of these three values are two, then we have a "perfect serpentine."
 *   Example:  AB BC AC     12 23 13     vals:  222 000 000
 *   Example:  AB BC ABC    12 23 123    vals:  232 000 000
 *   Example:  AB ABC ABC   12 123 123   vals:  332 000 000
 *   Example:  ABC ABC ABC  123 123 123  vals:  333 000 000
 *
 * The sums of the bitmasks for the four cells in an (M choose 4) serpentine
 * will have exactly four values set.  Those four values will each have
 * a value of two, three or four.
 * If all of these four values are two, then we have a "perfect serpentine."
 *   Example:  AB BC CD DA          12 23 34 14          bits:  222 200 000
 *   Example:  AB BCD CD DA         12 234 34 14         bits:  222 300 000
 *   Example:  ABD BCD CD DA        124 234 34 14        bits:  222 400 000
 *   Example:  ABCD BCD CD DA       1234 234 34 14       bits:  223 400 000
 *   Example:  ABCD BCD CD ACD      1234 234 34 134      bits:  224 400 000
 *   Example:  ABC BCD BCD AD       123 234 234 14       bits:  233 300 000
 *   Example:  ABCD BCD BCD AD      1234 234 234 14      bits:  233 400 000
 *   Example:  ABCD BCD BCD ACD     1234 234 234 134     bits:  234 400 000
 *   Example:  ABCD BCD BCD ABCD    1234 234 234 1234    bits:  244 400 000
 *   Example:  ABCD ABCD BCD ABCD   1234 1234 234 1234   bits:  344 400 000
 *   Example:  ABCD ABCD ABCD ABCD  1234 1234 1234 1234  bits:  444 400 000
 * 
 * Example (5 choose 4), serpentine:  35 123 135 1235:
 *  |     8 |     9 |    7  | 3 5   | 123 5 | 123   |    6  | 1 3 5 | 1 34  |
 * becomes (2 values cleared):
 *  |     8 |     9 |    7  | 3 5   | 123 5 | 123   |    6  | 1 3 5 |    4  |
 *
 * Example (5 choose 4), serpentine:  35 1357 1357 1357:
 *  |  4    | 23 5  | 3 5   |    6  | 1357  |     8 |     9 | 1357  | 1357  |
 * becomes (2 values cleared):
 *  |  4    | 2     | 3 5   |    6  | 1357  |     8 |     9 | 1357  | 1357  |
 *
 * Example (4 choose 4), serpentine:  135 137 357 357:
 *  | 12378 | 13 7  | 123478 |     9 | 1 3 5 | 12348 | 3 5 7 | 3 5 7 |    6  |
 * becomes (8 values cleared):
 *  |  2  8 | 13 7  |  2 4 8 |     9 | 1 3 5 |  2 48 | 3 5 7 | 3 5 7 |    6  |
 *
 * Example (4 choose 4), serpentine:  135 137 137 357:
 *  | 13  7 | 13679 | 13679 |     8 | 2     |  4    | 1 3 7 | 1 3 5 | 3 5 7 |
 * becomes (6 values cleared):
 *  | 13  7 |   6 9 |   6 9 |     8 | 2     |  4    | 1 3 7 | 1 3 5 | 3 5 7 |
 *
 * Example (4 choose 4), serpentine:  378 378 678 3678:
 *  | 23678 | 3 678 |     9 |  4    | 1     |   5   | 3  78 | 3  78 |   678 |
 * becomes (4 values cleared):
 *  | 2     | 3 678 |     9 |  4    | 1     |   5   | 3  78 | 3  78 |   678 |
 *
 * Example (5 choose 3), serpentine:  39 36 69:
 *  |  4    | 2     | 3   9 | 1     | 3 567 | 3  6  |  5 78 |  6  9 | 3 5 8 |
 * becomes (3 values cleared):
 *  |  4    | 2     | 3   9 | 1     |   5 7 | 3  6  |  5 78 |  6  9 |   5 8 |
 *
 * Example (4 choose 3), serpentine:  16 69 19:
 *  | 13 67 | 3  67 |   5   |     8 | 2     | 1   9 | 1  6  |  4    |   6 9 |
 * becomes (3 values cleared):
 *  |  3  7 | 3   7 |   5   |     8 | 2     | 1   9 | 1  6  |  4    |   6 9 |
 *
 * Example (4 choose 3), serpentine:  46 68 48:
 *  |    7  | 13 69 |   6 8 |   5   | 34  9 |  4  8 | 2     | 1 346 |  4 6  |
 * becomes (4 values cleared):
 *  |    7  | 13  9 |   6 8 |   5   | 3   9 |  4  8 | 2     | 1 3   |  4 6  |
 *
 * Example (3 choose 3), serpentine:  46 68 48:
 *  | 3     | 24 67 | 24 67 |   6 8 | 4   8 | 4 6   | 1     |     9 |   5   |
 * becomes (4 values cleared):
 *  | 3     | 2   7 | 2   7 |   6 8 | 4   8 | 4 6   | 1     |     9 |   5   |
 *
 * Information below is from earlier testing.  It is still believed to
 * be correct.
 *
 * Note:  d6.txt contains a perfect serpentine:  34 23 24
 * Note:  d6.txt contains a perfect serpentine:  56 25 26, which leads to
 *   four values being cleared:
 *  |   56  |  4    | 3     | 125 9 | 125 9 | 2  5  |    7  |    8  | 2  6  |
 * becomes:
 *  |   56  |  4    | 3     | 1   9 | 1   9 | 2  5  |    7  |    8  | 2  6  |
 *
 * Note:  m3.txt contains a serpentine:  269 29 26, which leads to
 *   four values being cleared:
 *  | 2 6 9 | 3     | 2   9 |    7  | 126 8 |   5   | 2   6 | 126 8 |  4    |
 * becomes:
 *  | 2 6 9 | 3     | 2   9 |    7  | 1   8 |   5   | 2   6 | 1   8 |  4    |
 *
 * Note:  m4.txt contains a serpentine:  278 27 78, which leads to
 *   five values being cleared:
 *  | 24789 | 2 78  | 1     |   5   | 2  7  |  3    |    6  |    78 | 4 789 |
 * becomes:
 *  |  4  9 | 2 78  | 1     |   5   | 2  7  |  3    |    6  |    78 | 4   9 |
 *
 * Note:  m9.txt contains a serpentine:  46 146 146, which leads to
 *   four values being cleared:
 *  | 34 67 |  4 6  | 34 67 |     9 | 1 4 6 |   5   | 2     | 1 4 6 |    8  |
 * becomes:
 *  | 3   7 |  4 6  | 3   7 |     9 | 1 4 6 |   5   | 2     | 1 4 6 |    8  |
 *
 * Note:  m18.txt contains a serpentine:  568 568 568, which leads to
 *   four values being cleared:
 *  |  4    | 1     | 25 78 |     9 |  3    |  56 8 |  56 8 |  56 8 | 2 678 |
 *  |  4    | 1     | 2  7  |     9 |  3    |  56 8 |  56 8 |  56 8 | 2  7  |
 *
 * Note that serpentine analysis appears to have substantial overlap with
 *   modified tuple analysis.
 */
uint4
new_serpentine_analysis(cell_t *test_cells[DIMENSION_SIZE])
{
    uint4     num_changes;

    /* test for serpentines of the form (M choose 3) */
    num_changes = new_serpentine_m_choose_n_analysis(test_cells, 3);

    /* test for serpentines of the form (M choose 4) */
    num_changes = new_serpentine_m_choose_n_analysis(test_cells, 4);

    return (num_changes);
}

/*
 * (multi) dimensional analysis
 *     Within a square dimension, two or three undetermined cells may be
 *     the only cells which can possibly contain a particular value.  If
 *     those cells are also fully contained in another dimension (a row
 *     or a column), then the value is cleared in all other cells of that
 *     other dimension.
 *     Example:
 *       In the rightmost square, 369 and 69 represent the only possible 9s
 *       in that square.  One of them must contain 9.  Since both 369 and 69
 *       are also in the same row dimension, no other value in that row
 *       can contain 9.  So, 369 in the first column can be set to 36 
 * 
 *       | 3 69 | 2    | 1    |   5  |    7 |  4   |    8 | 3 69 |   69 |
 *       | 3 89 |   5  | 3 89 | 3  8 |   6  | 1    |  4   |   7  | 2    |
 *       |   7  |  4 8 | 3468 | 3  8 |    9 | 2    |   5  | 3 6  | 1    |
 *     becomes:
 *       | 3 6  | 2    | 1    |   5  |    7 |  4   |    8 | 3 69 |   69 |
 *       | 3 89 |   5  | 3 89 | 3  8 |   6  | 1    |  4   |   7  | 2    |
 *       |   7  |  4 8 | 3468 | 3  8 |    9 | 2    |   5  | 3 6  | 1    |
 *
 * Must be called for a square dimension only.
 *
 * Fixme -- Believed to work correctly, but more testing is indicated
 */
uint4
dimensional_analysis(board_t *board, cell_t *test_cells[DIMENSION_SIZE])
{
    cell_t    *dim_test_cells[DIMENSION_SIZE];
    uint4     cell_index;
    cell_t    *cell;
    uint4     val, val_cnt;

    uint4     row, col, dimension;
    uint4     num_changes = 0;
    bool      rc;

    /* Fixme:  Need to do this? */
    unmark_all_cells(test_cells);

    /* For each cell in this dimension */
    for (cell_index=0; cell_index < DIMENSION_SIZE; cell_index++) {
        cell = test_cells[cell_index];
        if (cell->num_possible_values == 1) {
            continue;
        }

        /* For each possible value in this cell */
        for (val=0; val < DIMENSION_SIZE; val++) {
            if (!cell->values[val]) {
                continue;
            }
            val_cnt = mark_all_cells_containing_value(test_cells, val);
            if (val_cnt < 2 || val_cnt > NUM_SIDE_CELLS_IN_SQUARE) {
                unmark_all_cells(test_cells);
                continue;
            }

            /*
             * TRUE if all the marked cells in the square are also contained
             * in a row or column dimension.  If TRUE, also returns the
             * row and column coordinates of the dimension, and which
             * dimension it is.
             */
            rc = all_marked_cells_in_same_dimension(test_cells, &row, &col,
                &dimension);
            if (rc) {
                printd(D_DIM, ">>>>> Dim (%u, %u, %u) [%u]:  ",
                    cell->row, cell->col, cell->square, SQUARE);
		printd(D_DIM, "row %d, col %d, dim %d, ci %d, val %d; ",
                    row, col, dimension, cell_index, val + 1);
                printd(D_DIM, " match:  |");
                printd_cell_cell(D_DIM, cell);
                printd(D_DIM, " <<<\n");

                printd_dimension(D_DIM, test_cells);

                /* Load the row or column dimension identified above */
                rc = get_relevant_cells(board, row, col, dimension,
                    dim_test_cells);

                /*
                 * All cells containing "val" within the square dimension are
                 * marked.  Those marked cells are also fully contained in a
                 * row or column dimension.  Since one of the marked cells
                 * in the square dimension must contain "val", "val" cannot
                 * exist in the balance of the row or column dimension.  Clear
                 * "val" anywhere it may appear in the balance of the row or
                 * column dimension.
                 *
                 * Due to the magic of pointer arrays, marked cells in the
                 * square dimension are also marked in the row or column
                 * dimension.
                 */
                num_changes += clear_val_in_unmarked_cells_in_dimension(
                    dim_test_cells, val, row, col, dimension);
            }

            unmark_all_cells(test_cells);
        }
    }

    return (num_changes);
}

/*
 * Apply all solver algorithms to a single dimension (row, column
 *   or square) in the game
 * game:  Game structure.
 * row:  Zero-based row index
 * col:  Zero-based column index
 * dimension:  ROW, COL or SQUARE
 */
uint4
process_one_dimension(board_t *board, uint4 row, uint4 col, uint4 dimension)
{
    cell_t    *test_cells[DIMENSION_SIZE];
    uint4     num_changes = 0;

    /*
     * Create a pointer array for the cells in this dimension.
     */
    get_relevant_cells(board, row, col, dimension, test_cells);

    //print_array(test_val);
    //printf("\n");

    /*
     * For every undetermined cell in this dimension, find values
     * those cells cannot contain (values from determined cells)
     * and eliminate them.
     */
    num_changes += cannot_be_analysis(test_cells, dimension);

    /*
     * If an undetermined cell contains a value that is
     * unique in this dimension, set it to that value.
     */
    num_changes += must_be_analysis(test_cells);

    /*
     * If two undetermined cells contain two values, and
     * those values are the same, then those two values
     * can be eliminated from all other cells in the
     * dimension.
     */
    num_changes += pair_analysis(test_cells, dimension);

    /*
     * If three undetermined cells contain the same three
     * values, or two of the cells are subtuples of the
     * third, those three values can be eliminated from
     * all other cells in the dimension.
     */
    num_changes += modified_tuple_analysis(test_cells, dimension);

    /*
     * If this is a square, perform dimensional analysis.
     *
     * Fixme:  Row or column changes are credited to square.
     */
    if (dimension == SQUARE) {
        num_changes += dimensional_analysis(board, test_cells);
    }

    /*
     * If three undetermined cells contain two or three values that
     * overlap in a serpentine pattern, those three values can be
     * eliminated from all other cells in the dimension.
     *
     * New serpentine analysis implements (M choose N) testing.
     */
    //num_changes += serpentine_analysis(test_cells);
    num_changes += new_serpentine_analysis(test_cells);

    return (num_changes);
}

/*
 * Apply all solver algorithms to every row in the board.
 */
uint4
process_rows(board_t *board)
{
    uint4    row;
    uint4    num_changes = 0;

    for (row=0; row < DIMENSION_SIZE; row++) {
        num_changes += process_one_dimension(board, row, 0, ROW);
    }

    return (num_changes);
}

/*
 * Apply all solver algorithms to every column in the board.
 */
uint4
process_cols(board_t *board)
{
    uint4    col;
    uint4    num_changes = 0;

    for (col=0; col < DIMENSION_SIZE; col++) {
        num_changes += process_one_dimension(board, 0, col, COL);
    }

    return (num_changes);
}

/*
 * Apply all solver algorithms to every square in the board.
 */
uint4
process_squares(board_t *board)
{
    uint4    row, col;
    uint4    num_changes = 0;

    for (row=0; row < NUM_VERT_CELLS; row += NUM_VERT_CELLS_IN_SQUARE) {
        for (col=0; col < NUM_HORIZ_CELLS; col += NUM_HORIZ_CELLS_IN_SQUARE) {
            num_changes += process_one_dimension(board, row, col, SQUARE);
        }
    }

    return (num_changes);
}

/*
 * Apply all solver algorithms to every row, column and square in the board.
 * Continue to iterate until no further changes are made by the solver
 * algorithms.
 */
uint4
process_board(game_t *game, board_t *board, uint4 *sum_num_row_changes,
              uint4 *sum_num_col_changes, uint4 *sum_num_square_changes,
              uint4 *iterations)
{
    uint4    pb_num_row_changes = 0;
    uint4    pb_num_col_changes = 0;
    uint4    pb_num_square_changes = 0;
    uint4    pb_num_changes = 0;
    uint4    pb_tot_num_changes = 0;

    do {
        pb_num_row_changes = process_rows(board);
        *sum_num_row_changes += pb_num_row_changes;
        if (0 == game->dprint.silent_level) {
            printf("Rows:  %u changes\n", pb_num_row_changes);
        }

        pb_num_col_changes = process_cols(board);
        *sum_num_col_changes += pb_num_col_changes;
        if (0 == game->dprint.silent_level) {
            printf("Cols:  %u changes\n", pb_num_col_changes);
        }

        pb_num_square_changes = process_squares(board);
        *sum_num_square_changes += pb_num_square_changes;
        if (0 == game->dprint.silent_level) {
            printf("Squares:  %u changes\n", pb_num_square_changes);
        }

        (*iterations)++;
        pb_num_changes =
            (pb_num_row_changes + pb_num_col_changes + pb_num_square_changes);
        pb_tot_num_changes += pb_num_changes;

    } while (pb_num_changes != 0);

    return (pb_tot_num_changes);
}

/*
 * Breadth-first, one-level recursion.
 * Iteratively:
 * - finds the first (or next) undetermined cell
 *   - "guesses" (selects) a value for that undetermined cell, from among
 *     the possible values for that cell
 *     - applies all solver algorithms to every row, column and square in
 *       the board.  Continues to iterate until no further changes are
 *       made by the solver algorithms
 *     - if the solver is unsuccessful, repeats the above two steps
 *   - if the above three steps are unsuccessful, returns FALSE
 *   - if the above three steps succeed, returns TRUE
 */
uint4
breadth_first_1lr_process_board(game_t *game, board_t *board,
                                uint4 *sum_num_row_changes,
                                uint4 *sum_num_col_changes,
                                uint4 *sum_num_square_changes,
                                uint4 *sum_num_tot_changes,
                                uint4 *iterations,
                                uint4 *sum_cells_tested,
                                uint4 *sum_values_tested)
{
    board_t  *board_new;
    cell_t   *cell;
    uint4    value;
    uint4    bf_tot_num_row_changes = 0;
    uint4    bf_tot_num_col_changes = 0;
    uint4    bf_tot_num_square_changes = 0;
    uint4    bf_iterations = 0;
    uint4    bf_tot_changes = 0;
    bool     solved = FALSE;;
    bool     sane = FALSE;;
    bool     bf_success = FALSE;;
    uint4    bf_cells_tested = 0;
    uint4    bf_values_tested = 0;
    bfnuc_state_t  bfnuc_state;

    // Fixme:  Redundant at the moment.
    *sum_cells_tested  = 0;
    *sum_values_tested = 0;

    /*
     * Make a copy of the board so we can restore it later, if necessary
     * Update the pointers to correctly link board_curr to board_new,
     * and board_new to board_curr.  Also, set game pointer in board_new.
     */
    board_new = malloc(sizeof(board_t));
    if (!board_new) {
        return (1);
    }
    board_copy(board_new, board);
    board_set_pointers(game->board_curr, game, game->board_curr->prev,
        board_new);
    board_set_pointers(board_new, game, game->board_curr, NULL);

    /*
     * bfnuc_state contains state for board_find_next_undetermined_cell()
     * We can't mess with it after initialization.
     */
    bfnuc_state.state_index = board_get_initial_cell_index(game);
    bfnuc_state.state_count = 0;

    /*
     * For every undetermined cell,
     *   For every possible value in that undetermined cell,
     *     - set that value in that cell
     *     - run the iterative algorithms over the modified board
     *       - if puzzle solved, return SOLVED_SUCCESS
     *       - if puzzle not solved, continue
     *     - if no more undetermined cells, return SOLVED_FAIL
     */
    do {
        cell = board_find_next_undetermined_cell(game, board_new, &bfnuc_state);
        if (cell) {
            printd(D_BFS, "BFS:  undetermined cell:  (%u, %u, %u):  |",
                cell->row, cell->col, cell->square);
            printd_cell_cell(D_BFS, cell);
            printd(D_BFS, "  cell_index %d\n", bfnuc_state.state_index);
            bf_cells_tested++;
        } else {
            printd(D_BFS, "BFS:  cell not found, solution FAILED!\n");
            bf_success = FALSE;

            goto search_complete;
        }
        /*
         * Undetermined cell found:  Process it!
         */
        for (value=1; value <= DIMENSION_SIZE; value++) {
            if (cell_value_set(cell, value)) {
                /*
                 * Value in undetermined cell found:  Process it!
                 */
                printd(D_BFS, "  bfs:  value found!  %d\n", value);
                bf_values_tested++;

                /*
                 * Modify the original cell to contain only "value".
                 * "value" is one of the values that this undetermined
                 * cell could represent.  "value" is effectively our
                 * guess as to the value this cell should represent.
                 */
                set_cell_value(cell, value);

                printd(D_BFS, "    bfs:  cell now:  (%u, %u, %u):  |",
                    cell->row, cell->col, cell->square);
                printd_cell_cell(D_BFS, cell);
                printd(D_BFS, "  cell_index %d\n", bfnuc_state.state_index);

                if (0 == game->dprint.silent_level) {
                    printd_board(D_BFS, board_new);
                }

                bf_tot_num_row_changes = 0;
                bf_tot_num_col_changes = 0;
                bf_tot_num_square_changes = 0;
                bf_iterations = 0;

                bf_tot_changes = process_board(game, board_new,
                    &bf_tot_num_row_changes, &bf_tot_num_col_changes,
                    &bf_tot_num_square_changes, &bf_iterations);

                if (0 == game->dprint.silent_level) {
                    printd_board(D_BFS, board_new);
                }

                if (bf_tot_changes) {
                    printd(D_BFS, "    bfs:  bf tot_changes %d\n",
                        bf_tot_changes);

                    solved = board_is_solved(board_new);
                    printd(D_BFS, "    bfs:  bf solved %d\n", solved);

                    sane = FALSE;
                    if (solved) {
                        sane = board_is_sane_solved(board_new);
                    }
                    printd(D_BFS, "    bfs:  bf sane %d\n", sane);

                    if (solved && sane) {
                        printd(D_BFS, "    BFS:  bf SUCCESS!!!\n");
                        bf_success = TRUE;

                        goto search_complete;
                    }
                }

                /* Restore the board to its original state */
                board_copy_preserve_pointers(board_new, board);
            }
        }
    } while (TRUE);

    /*
     * We got here if:
     * - all undetermined cells have been tested, unsuccessfully, and we
     *   have no more undetermined cells to test
     * - we were successful; by setting a value in a single undetermined
     *   cell, we were able to generate a solved & sane solution
     */
search_complete:
    if (0 == game->dprint.silent_level) {
        printf("BFS:  %s %s%s!\n", game->filename,
            (solved ? "solved" : "not solved"),
            (sane ? ", sane" : ""));
        printf("    BFS:  Iter %u; "
            "Chg:  Row %u col %u sq %u tot %u cell %u val %u\n",
            bf_iterations, bf_tot_num_row_changes, bf_tot_num_col_changes,
            bf_tot_num_square_changes, bf_tot_changes,
            bf_cells_tested, bf_values_tested);
    }

    if (0 == game->dprint.silent_level) {
        printd_board(D_BFS, board_new);
    }

    if (bf_success) {
        /*
         * On success, the current board becomes the new, freshly solved
         * board.  All of the other pointers are already correct.
         */
        game->board_curr = board_new;

        /* Add BFS-caused changes to the running totals kept by the game */
        *sum_num_row_changes    += bf_tot_num_row_changes;
        *sum_num_col_changes    += bf_tot_num_col_changes;
        *sum_num_square_changes += bf_tot_num_square_changes;
        *sum_num_tot_changes    += bf_tot_changes;
        *iterations             += bf_iterations;
    } else {
        /*
         * On failure, we need to free the new board and put the pointers
         * back the way they were when we were called.  There may be further
         * processing that we need to accommodate.
         * Fixme:  Delay initial setting of pointers until the end?
         */
        board_set_pointers(game->board_curr, game, game->board_curr->prev,
            NULL);
        free(board_new);
    }

    /* Pass or fail, report BFS-specific changes to the caller */
    *sum_cells_tested        += bf_cells_tested;
    *sum_values_tested       += bf_values_tested;

    return (bf_success);
}

/*
 * Depth-first recursion.
 *
 * Iteratively:
 * - finds the first (or next) undetermined cell
 *   - "guesses" (selects) a value for that undetermined cell, from among
 *     the possible values for that cell
 *     - applies all solver algorithms to every row, column and square in
 *       the board.  Continues to iterate until no further changes are
 *       made by the solver algorithms
 *     - if the puzzle is solved, and sane, returns SOLVED_SUCCESS
 *     - if puzzle is not solved, and is not sane, tries the next
 *       guess for this board (the next undetermined cell and/or
 *       the next value to try)
 *     - if the puzzle is not solved, but is sane, calls this function
 *       recursively
 *   - if we are out of guesses (out of undetermined cells to test),
 *     returns rrs_no_more_cells
 */
enum recurse_return_status
depth_first_process_board(game_t *game, board_t *board_orig,
                          board_t **board_ret,
                          recurse_stats_t *stats_ret,
                          uint4 recursion_level)
{
    board_t  *board_new;
    cell_t   *cell;
    uint4    value;
    uint4    df_tot_num_row_changes = 0;
    uint4    df_tot_num_col_changes = 0;
    uint4    df_tot_num_square_changes = 0;
    uint4    df_iterations = 0;
    uint4    df_tot_changes = 0;
    bool     solved = FALSE;;
    bool     sane = FALSE;;
    uint4    df_cells_tested = 0;
    uint4    df_values_tested = 0;
    bfnuc_state_t  bfnuc_state;
    enum recurse_return_status  df_status = rrs_initial_error;;

    // Fixme:  board_ret believed unused, unnecessary.
    *board_ret = NULL;

    /* Record the maximum recursion depth in the stats structure */
    if (recursion_level > stats_ret->max_recursion_levels) {
        stats_ret->max_recursion_levels = recursion_level;
    }

    recursion_level++;

    /* Limit the number of levels of recursion */
    if (recursion_level > game->max_num_recursion_levels) {
        return (rrs_depth_error);
    }

    /*
     * Make a copy of the board so we can restore it later, if necessary
     * Update the pointers to correctly link board_curr to board_new,
     * and board_new to board_curr.  Also, set game pointer in board_new.
     */
    board_new = malloc(sizeof(board_t));
    if (!board_new) {
        return (rrs_system_error);
    }
    board_copy(board_new, board_orig);
    board_set_pointers(board_orig, game, board_orig->prev, board_new);
    board_set_pointers(board_new, game, board_orig, NULL);

    /*
     * bfnuc_state contains state for board_find_next_undetermined_cell()
     * We can't mess with it after initialization.
     */
    bfnuc_state.state_index = board_get_initial_cell_index(game);
    bfnuc_state.state_count = 0;

    /*
     * For every undetermined cell,
     *   For every possible value in that undetermined cell,
     *     - set that value in that cell
     *     - run the iterative algorithms over the modified board
     *       - if puzzle solved, return SOLVED_SUCCESS
     *       - if puzzle not solved, and not sane, continue
     *       - if puzzle not solved, but is sane, call this function
     *         recursively
     *     - if no more undetermined cells, return rrs_no_more_cells
     */
    do {
        cell = board_find_next_undetermined_cell(game, board_new, &bfnuc_state);
        if (cell) {
            printd(D_DFS, "DFS:  undetermined cell:  (%u, %u, %u):  |",
                cell->row, cell->col, cell->square);
            printd_cell_cell(D_DFS, cell);
            printd(D_DFS, "  cell_index %d\n", bfnuc_state.state_index);
            df_cells_tested++;
        } else {
            printd(D_DFS, "DFS:  cell not found, cells exhausted!\n");
            df_status = rrs_no_more_cells;

            goto search_complete;
        }
        /*
         * Undetermined cell found:  Process it!
         */
        for (value=1; value <= DIMENSION_SIZE; value++) {
            if (cell_value_set(cell, value)) {
                /*
                 * Value in undetermined cell found:  Process it!
                 */
                printd(D_DFS, "  dfs:  value found!  %d\n", value);
                df_values_tested++;

                /*
                 * Modify the original cell to contain only "value".
                 * "value" is one of the values that this undetermined
                 * cell could represent.  "value" is effectively our
                 * guess as to the value this cell should represent.
                 */
                set_cell_value(cell, value);

                printd(D_DFS, "    dfs:  cell now:  (%u, %u, %u):  |",
                    cell->row, cell->col, cell->square);
                printd_cell_cell(D_DFS, cell);
                printd(D_DFS, "  cell_index %d\n", bfnuc_state.state_index);

                if (0 == game->dprint.silent_level) {
                    printd_board(D_DFS, board_new);
                }

                df_tot_num_row_changes = 0;
                df_tot_num_col_changes = 0;
                df_tot_num_square_changes = 0;
                df_iterations = 0;

                df_tot_changes = process_board(game, board_new,
                    &df_tot_num_row_changes, &df_tot_num_col_changes,
                    &df_tot_num_square_changes, &df_iterations);

                if (0 == game->dprint.silent_level) {
                    printd_board(D_DFS, board_new);
                }

                /*
                 * Non-zero if process_board() was able to made changes,
                 * meaning that it was able to make forward progress in
                 * solving the puzzle.
                 */
                if (df_tot_changes) {
                    printd(D_DFS, "    dfs:  df tot_changes %d\n",
                       df_tot_changes);

                    solved = board_is_solved(board_new);
                    printd(D_DFS, "    dfs:  df solved %d\n", solved);

                    if (solved) {
                        sane = board_is_sane_solved(board_new);
                    } else {
                        sane = board_is_sane_part_solved(board_new);
                    }
                    printd(D_DFS, "    dfs:  df sane %d\n", sane);

                    if (solved && sane) {
                        printd(D_DFS, "    DFS:  df SUCCESS!!!\n");
                        df_status = rrs_success;
                        /* Only update this on initial success */
                        game->board_curr = board_new;

                        goto search_complete;
                    } else if (sane) {
                        /*
                         * Sane but not solved:  Recurse here.
                         */
                        printd(D_DFS2, ">>>>>>>> recurse %d! <<<<<<<<\n",
                          recursion_level);
                        stats_ret->tot_num_recursions++;
                        df_status = depth_first_process_board(game,
                            board_new, board_ret, stats_ret, recursion_level);
                        if (rrs_success == df_status) {
                            printd(D_DFS2, ">>>>>>>> recurse return success "
                              "%d! %d <<<<<<<<\n", recursion_level,
                              df_status);
                            board_new = game->board_curr;

                            solved = board_is_solved(board_new);
                            printd(D_DFS, "    dfs:  df rec solved %d\n",
                              solved);

                            if (solved) {
                                sane = board_is_sane_solved(board_new);
                            } else {
                                sane = board_is_sane_part_solved(board_new);
                            }
                            printd(D_DFS, "    dfs:  df rec sane %d\n", sane);

                            goto search_complete;
                        } else {
                            stats_ret->tot_num_backtracks++;
                            //printd(D_DFS2, ">>>>>>>> recurse return failure "
                            //  "%d! %d <<<<<<<<\n", recursion_level,
                            //  df_status);
                        }
                    } else {
                        /* not sane, not solved */
                        stats_ret->tot_num_not_sane++;
                    }
                }

                /* Restore the board to its original state */
                board_copy_preserve_pointers(board_new, board_orig);
            }
        }
    } while (TRUE);

    /*
     * We got here if:
     * - all undetermined cells have been tested, unsuccessfully, and we
     *   have no more undetermined cells to test
     * - we were successful; by (recursively, once per level) setting a
     *   value in an undetermined cell, we were able to generate a
     *   solved & sane solution
     */
search_complete:
    if (0 == game->dprint.silent_level) {
        printf("DFS:  %s %s%s!\n", game->filename,
            (solved ? "solved" : "not solved"),
            (sane ? ", sane" : ", not sane"));
        printf("    DFS:  Iter %u; "
            "Chg:  Row %u col %u sq %u tot %u cell %u val %u\n",
            df_iterations, df_tot_num_row_changes, df_tot_num_col_changes,
            df_tot_num_square_changes, df_tot_changes,
            df_cells_tested, df_values_tested);
    }

    if (0 == game->dprint.silent_level) {
        printd_board(D_DFS, board_new);
    }

    if (rrs_success == df_status) {
        /*
         * On success, above, the current board of the game was set to be
         * the new, freshly solved board.  All of the other board pointers
         * are already correct.
         * rrs_success means the board is solved, and sane.
         */

        /* Add DFS-caused changes to the running totals kept by the game */
        stats_ret->sum_num_row_changes    += df_tot_num_row_changes;
        stats_ret->sum_num_col_changes    += df_tot_num_col_changes;
        stats_ret->sum_num_square_changes += df_tot_num_square_changes;
        stats_ret->sum_num_tot_changes    += df_tot_changes;
        stats_ret->iterations             += df_iterations;
    } else {
        /*
         * On failure, we need to free the new board and put the pointers
         * back the way they were when we were called.  There may be further
         * processing that we need to accommodate.
         * Fixme:  Delay initial setting of pointers until the end?
         */
        board_set_pointers(board_orig, game, board_orig->prev, NULL);
        free(board_new);
    }

    /* Pass or fail, report DFS-specific changes to the caller */
    stats_ret->sum_cells_tested  += df_cells_tested;
    stats_ret->sum_values_tested += df_values_tested;

    return (df_status);
}

/*
 * main()
 */
int
main(int argc, char **argv)
{
    game_t   *game;
    board_t  *board_ret = NULL;
    uint4    tot_changes;
    uint4    tot_num_row_changes = 0;
    uint4    tot_num_col_changes = 0;
    uint4    tot_num_square_changes = 0;
    uint4    iterations = 0;
    int      rc;
    bool     solved;
    bool     sane;
    bool     bfs_success;
    bool     do_bfs = FALSE;
    uint4    bfs_num_cells_tested = 0;
    uint4    bfs_num_values_tested = 0;
    enum recurse_return_status     dfs_success;
    bool     do_dfs = FALSE;

    /*
     * Initialize data structures
     */
    game = sudoku_init();
    if (!game) {
        printf("\n*** Initialization error!  Exiting!\n");
        exit(10);
    }

    g_dprint = &(game->dprint);

    rc = input_values(argc, argv, game);
    if (rc) {
        if (0 > rc) {
            printf("\n*** Input error %d!  Exiting!\n", rc);
        }
        sudoku_cleanup(game);
        exit(11);
    }

    if (1 >= game->dprint.silent_level) {
        print_board(game->board_curr);
    }

    sane = board_is_sane_never_solved(game->board_curr);
    if (!sane) {
        printf("\n*** Initial board is not sane!  Exiting!\n");
        sudoku_cleanup(game);
        exit(12);
    }

    tot_changes = process_board(game, game->board_curr, &tot_num_row_changes,
        &tot_num_col_changes, &tot_num_square_changes, &iterations);

    solved = board_is_solved(game->board_curr);
    if (solved) {
        sane = board_is_sane_solved(game->board_curr);
    } else {
        sane = board_is_sane_part_solved(game->board_curr);
    }

    if (0 == game->dprint.silent_level) {
        printf("ALG:  %s %s%s!\n", game->filename,
            (solved ? "solved" : "not solved"),
            (sane ? ", sane" : ""));
        printf("    Iterations:  %u.  "
            "Changes:  Rows %u, cols %u, squares %u, tot %u\n",
            iterations, tot_num_row_changes, tot_num_col_changes,
            tot_num_square_changes, tot_changes);
    }

    if (0 == game->dprint.silent_level) {
        print_board(game->board_curr);
    }

    /*
     * Only do a breadth-first search with limited guessing if:
     *   - it was requested on the command line
     *   - the puzzle was not already solved, above
     */
    bfs_success = FALSE;
    if ((BREADTH_FIRST_1LR & game->game_flags) && !(solved && sane)) {
        do_bfs = TRUE;
        bfs_success = breadth_first_1lr_process_board(game,
            game->board_curr, &tot_num_row_changes, &tot_num_col_changes,
            &tot_num_square_changes, &tot_changes, &iterations,
            &bfs_num_cells_tested, &bfs_num_values_tested);
    }

    if (bfs_success) {
        solved = board_is_solved(game->board_curr);
        if (solved) {
            sane = board_is_sane_solved(game->board_curr);
        } else {
            sane = board_is_sane_part_solved(game->board_curr);
        }
    }

    if (BREADTH_FIRST_1LR & game->game_flags) {
        /* BFS:  Pass or fail, print the board if enabled */
        if (0 == game->dprint.silent_level) {
            print_board(game->board_curr);
        }
    }

    /*
     * Only do a depth-first search with generalized guessing if:
     *   - it was requested on the command line
     *   - the puzzle was not already solved, above
     */
    dfs_success = rrs_initial_error;
    if ((REC_DESCENT & game->game_flags) && !(solved && sane)) {
        do_dfs = TRUE;
        game->recurse_stats = alloc_init_game_stats();
        dfs_success = depth_first_process_board(game, game->board_curr,
            &board_ret, game->recurse_stats, 0);
    }

    /* rrs_success actually implies the board is solved, and sane */
    if (rrs_success == dfs_success) {
        solved = board_is_solved(game->board_curr);
        if (solved) {
            sane = board_is_sane_solved(game->board_curr);
        } else {
            sane = board_is_sane_part_solved(game->board_curr);
        }
        /* Add DFS-caused changes to the running totals kept by the game */
        tot_num_row_changes    += game->recurse_stats->sum_num_row_changes;
        tot_num_col_changes    += game->recurse_stats->sum_num_col_changes;
        tot_num_square_changes += game->recurse_stats->sum_num_square_changes;
        tot_changes            += game->recurse_stats->sum_num_tot_changes;
        iterations             += game->recurse_stats->iterations;
    }

    if (REC_DESCENT & game->game_flags) {
        /* DFS:  Pass or fail, print the board if enabled */
        if (0 == game->dprint.silent_level) {
            print_board(game->board_curr);
        }
    }

    if (1 >= game->dprint.silent_level) {
        print_board(game->board_curr);
    }

    if (2 >= game->dprint.silent_level) {
        printf("%s %s %s%s!  %s\n",
            ((solved && sane) ? ">>>>>>" : "***"),
            game->filename,
            (solved ? "solved" : "not solved"),
            (sane ? ", sane" : ", not sane"),
            ((solved && sane) ? "" : "Solution failed!"));
        printf("    Iterations:  %u.  "
            "Changes:  Rows %u, cols %u, squares %u, tot %u\n",
            iterations, tot_num_row_changes, tot_num_col_changes,
            tot_num_square_changes, tot_changes);
        if (do_bfs) {
            printf("        bfs:  cells %u, values %u\n", 
                bfs_num_cells_tested, bfs_num_values_tested);
        }
        if (do_dfs) {
            printf("        dfs:  cells %u vals %u rec %u bt %u ns %u mrl %u\n",
                game->recurse_stats->sum_cells_tested, 
                game->recurse_stats->sum_values_tested,
                game->recurse_stats->tot_num_recursions,
                game->recurse_stats->tot_num_backtracks,
                game->recurse_stats->tot_num_not_sane,
                game->recurse_stats->max_recursion_levels);
        }
    }

    sudoku_cleanup(game);

    if (!(solved && sane)) {
        exit(1);
    } else {
        exit(0);
    }
}

