/*
 * Conditional debug printing for C language Sudoku solver
 *
 * Steve Jacobson, 04/07/18
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

/*
 * Initialize various flags and optional logging
 */
void
printd_init(dprint_t *dprint, char *log_filename)
{
    dprint->log_filename = log_filename;
    dprint->log_fd       = NULL;
    // Fixme:  Magic number.  Disables most assigned debug flags.
    dprint->flags        = 0xff00fcc0;
    dprint->silent_level = 0;

    if (dprint->log_filename) {
        dprint->log_fd   = fopen(dprint->log_filename, "w");
    }
}

/*
 * Close logging
 */
void
printd_cleanup(dprint_t *dprint)
{
    if (dprint->log_fd) {
        fclose(dprint->log_fd);
    }
}

/*
 * Conditional debug printing with optional logging.
 */
void
printd(uint4 flags, const char *fmt, ...)
{
    va_list   ap;

    if (!(flags & g_dprint->flags)) {
        return;
    }

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    if (g_dprint->log_fd) {
        va_start(ap, fmt);
        vfprintf(g_dprint->log_fd, fmt, ap);
        va_end(ap);
    }
}

/*
 * Print the numerical representation of each TRUE value in a
 * values array.  This array is as that contained in a cell_t.
 * Note that for a standard 9x9 Sudoku game, array indices
 * 0-8 represent values 1-9.
 * Same as print_array(), but enables optional logging.
 */
void
printd_array(uint4 flags, bool array[DIMENSION_SIZE])
{
    uint4    i;

    for (i=0; i < DIMENSION_SIZE; i++) {
        if (array[i]) {
            printd(flags, "%u", i + 1);
        } else {
            printd(flags, " ");
        }
    }
}

/*
 * Print the numerical representation of each TRUE value in a
 * cell values array.
 */
void
printd_cell_cell_vals(uint4 flags, cell_t *cell)
{
    printd_array(flags, cell->values);
}

/*
 * Print the numerical representation of each TRUE value in a
 * cell values array, framed for board representation.
 */
void
printd_cell_cell(uint4 flags, cell_t *cell)
{
    printd(flags, " ");
    printd_cell_cell_vals(flags, cell);
    printd(flags, " |");
}

/*
 * Print the numerical representation of each TRUE value in a
 * cell values array, framed for board representation.
 */
void
printd_cell(uint4 flags, board_t *board, uint4 row, uint4 col)
{
    cell_t    *cell;

    cell = &board->x_y_board[row][col];

    printd_cell_cell(flags, cell);
}

/*
 * Print a representation of an entire game board.
 */
void
printd_board(uint4 flags, board_t *board)
{
    uint4    row, col;

    printd(flags, "=======================================================");
    printd(flags, "======================================================\n");
    for (row=0; row < NUM_HORIZ_CELLS; row++) {
        printd(flags, "|");
        for (col=0; col < NUM_VERT_CELLS; col++) {
            printd_cell(flags, board, row, col);
        }
        printd(flags, "\n");
        if ((row % 3) == 2) {
            printd(flags,
                "=======================================================");
            printd(flags,
                "======================================================\n");
        }
    }
}

/*
 * For debugging
 * Fixme:  Could be used in print_board()
 */
void
printd_dimension(uint4 flags, cell_t *cells[DIMENSION_SIZE])
{
    uint4    i;

    printd(flags, "|");
    for (i=0; i < DIMENSION_SIZE; i++) {
        printd_cell_cell(flags, cells[i]);
    }
    printd(flags, "\n");
}

