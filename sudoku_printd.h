/*
 * Structures, defines and externs for the sudoku debug print functions.
 *
 * Steve Jacobson, 04/07/18
 *
 * Copyright (c) 1996-2018 by Steven A. Jacobson, all rights reserved.
 */


/*
 * Externs
 */

extern void printd_init(dprint_t *dprint, char *log_filename);
extern void printd_cleanup(dprint_t *dprint);
extern void printd(uint4 flags, const char *fmt, ...);
extern void printd_cell_cell(uint4 flags, cell_t *cell);
extern void printd_board(uint4 flags, board_t *board);
extern void printd_dimension(uint4 flags, cell_t *cells[DIMENSION_SIZE]);

