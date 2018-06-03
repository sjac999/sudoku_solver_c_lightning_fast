/*
 * Structures, defines and externs for the sudoku print functions.
 *
 * Steve Jacobson, 06/03/18
 *
 * Copyright (c) 1996-2018 by Steven A. Jacobson, all rights reserved.
 */


/*
 * Externs
 */

extern void print_cell_cell(cell_t *cell);
extern void print_cell(board_t *board, uint4 row, uint4 col);
extern void print_dimension(cell_t *cells[DIMENSION_SIZE]);
extern void print_board(board_t *board);

