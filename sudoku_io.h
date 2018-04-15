/*
 * Structures, defines and externs for the sudoku I/O functions.
 *
 * Steve Jacobson, 04/01/18
 *
 * Copyright (c) 1996-2018 by Steven A. Jacobson, all rights reserved.
 */


/*
 * Externs
 */
extern int print_puzzle_array(
                            uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS]);
extern int read_puzzle_file(FILE *input_fd,
                            uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS]);
extern int print_puzzle_file(char *str1, char *str2, char *str3,
                            uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS]);
extern int fprint_puzzle_file(FILE *output_fd,
                              char *str1, char *str2, char *str3,
                            uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS]);

