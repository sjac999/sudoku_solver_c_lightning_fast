/*
 * Structures, defines and externs for the sudoku I/O functions.
 *
 * Steve Jacobson, 04/01/18
 *
 * Copyright (c) 1996-2018 by Steven A. Jacobson, all rights reserved.
 */


/*
 * Format of the input file.
 */
enum input_file_format {
    input_standard    = 0,
    input_linear      = 1,
};

/*
 * Format of the output file.
 */
enum output_file_format {
    output_standard   = 0,
    output_linear     = 1,
};

/*
 * Externs
 */
extern int print_puzzle_array(
                            uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS]);
extern int read_puzzle_file(FILE *input_fd, enum input_file_format file_format,
                            uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS]);
extern int print_puzzle_file(char *str1, char *str2, char *str3,
                            uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS]);
extern int fprint_puzzle_file(FILE *output_fd,
                              enum output_file_format file_format,
                              char *str1, char *str2, char *str3,
                            uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS]);

