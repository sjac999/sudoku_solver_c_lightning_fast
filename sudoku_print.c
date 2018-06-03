/*
 * Various print functions for C language Sudoku solver
 *
 * Steve Jacobson, 06/03/18
 *
 * Copyright (c) 1996-2018 by Steven A. Jacobson, all rights reserved.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "sudoku.h"
#include "sudoku_printd.h"
#include "sudoku_print.h"

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

