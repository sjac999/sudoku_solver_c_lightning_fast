/*
 * Input/output functions for C language Sudoku solver
 *
 * Steve Jacobson, 01/05/06
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
#include "sudoku_io.h"
#include "sudoku_printd.h"


#define MAX_LINE_SIZE       256


/*
 * Print the puzzle values from an intermediate format array.
 * Fixme:  Full of magic numbers.
 * Fixme:  Will not work for puzzles larger than 9x9.
 */
int
fprint_puzzle_array(FILE *output_fd,
                    uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS])
{
    uint4    row;
    uint4    col;
    uint4    value;

    for (row=0; row < NUM_VERT_CELLS; row++) {
        for (col=0; col < NUM_HORIZ_CELLS; col++) {
            value = board_array[row][col];
            switch (value) {
            case 0:
                fprintf(output_fd, ".");
                break;
            case 1:
                fprintf(output_fd, "1");
                break;
            case 2:
                fprintf(output_fd, "2");
                break;
            case 3:
                fprintf(output_fd, "3");
                break;
            case 4:
                fprintf(output_fd, "4");
                break;
            case 5:
                fprintf(output_fd, "5");
                break;
            case 6:
                fprintf(output_fd, "6");
                break;
            case 7:
                fprintf(output_fd, "7");
                break;
            case 8:
                fprintf(output_fd, "8");
                break;
            case 9:
                fprintf(output_fd, "9");
                break;
            default:
                fprintf(output_fd, "?");
                break;
            }
            if (col != (NUM_HORIZ_CELLS - 1)) {
                fprintf(output_fd, ",");
            }
            if (((col % NUM_HORIZ_CELLS_IN_SQUARE) == 2) && 
              (col != (NUM_HORIZ_CELLS - 1))) {
                fprintf(output_fd, " ");
            }
        }
        fprintf(output_fd, "\n");
        if (((row % NUM_VERT_CELLS_IN_SQUARE) == 2) &&
          (row != (NUM_VERT_CELLS - 1))) {
            fprintf(output_fd, "#\n");
        }
    }

    return (0);
}

/*
 * Print the puzzle values from an intermediate format array.
 * Fixme:  Full of magic numbers.
 * Fixme:  Will not work for puzzles larger than 9x9.
 */
int
print_puzzle_array(uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS])
{
    int      rc;

    rc = fprint_puzzle_array(stdout, board_array);

    return (rc);
}

/*
 * Read the input file
 *
 * Fixme:  Will not work for boards larger than 9x9.
 * Fixme:  Magic numbers.
 */
int
read_puzzle_file(FILE *input_fd,
                 uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS])
{
    char     input_buf[MAX_LINE_SIZE];
    int      scnt;
    uint4    row;
    uint4    col;
    char     line_str[9][10];
    char     test_str[10];
    uint4    col_val;
    uint4    num_rows_read = 0;
    uint4    cindx;

    /*
     * Initialize all locations in the intermediate format array to 0,
     * which is equivalent to "." in the input file.
     */
    for (row=0; row < NUM_VERT_CELLS; row++) {
        for (col=0; col < NUM_HORIZ_CELLS; col++) {
            board_array[row][col] = 0;
        }
    }

    /*
     * Read the input file and populate the initial board in the game
     */
    while (fgets(input_buf, MAX_LINE_SIZE, input_fd)) {
        printd(D_INPUT, ">>>%s<<<\n", input_buf);
        if ((input_buf[0] == '#') || (input_buf[0] == '\n')) {
            printd(D_INPUT, "### Comment or blank line:  Disregarding\n");
            continue;
        }
        scnt = sscanf(input_buf, "%[0-9. ],%[0-9. ],%[0-9. ],"
             "%[0-9. ],%[0-9. ],%[0-9. ],%[0-9. ],%[0-9. ],%[0-9. ]",
             line_str[0], line_str[1], line_str[2],
             line_str[3], line_str[4], line_str[5],
             line_str[6], line_str[7], line_str[8]);
        printd(D_INPUT, "scnt = %d\n", scnt);

        if (scnt != NUM_HORIZ_CELLS) {
            fclose(input_fd);
            return (-2);
        }
        if (num_rows_read >= NUM_VERT_CELLS) {
            fclose(input_fd);
            return (-3);
        }

        for (col=0; col < NUM_HORIZ_CELLS; col++) {
            cindx = 0;
            /* skip all leading ' ' if present */
            while (line_str[col][cindx] == ' ') {
                cindx++;
            }

            /* read and disregard "." entries */
            scnt = sscanf(&line_str[col][cindx], "%[.]", test_str);
            if (scnt == 1) {
                printd(D_INPUT, "    scnt = %d, . found\n", scnt);
                continue;
            }
            /* read and store "0-9" entries */
            scnt = sscanf(line_str[col], "%u", &col_val);
            printd(D_INPUT, "    scnt = %d, col_val = %u\n", scnt, col_val);
            if (scnt != 1) {
                fclose(input_fd);
                return (-4);
            }
            if ((col_val < 1) || (col_val > NUM_HORIZ_CELLS)) {
                fclose(input_fd);
                return (-5);
            }
            /*
             * Save the range-checked value into the intermediate format
             * array.  All unspecified cells will be set to 0.
             */
            board_array[num_rows_read][col] = (char)col_val;
        }
        num_rows_read++;
    }

    return (0);
}

/*
 * Output to a file:
 * - the file header
 * - the puzzle values from an intermediate format array
 */
int
fprint_puzzle_header(FILE *output_fd, char *str1, char *str2, char *str3)
{
    if (!str1 || !str2 || !str3) {
        return (-1);
    }

    fprintf(output_fd, "#\n");
    fprintf(output_fd, "# %s\n", str1);
    fprintf(output_fd, "# %s\n", str2);
    fprintf(output_fd, "# %s\n", str3);
    fprintf(output_fd, "#\n");

    return (0);
}

/*
 * Output to a file:
 * - the file header, constructed from the passed-in strings
 * - the puzzle values from an intermediate format array
 */
int
fprint_puzzle_file(FILE *output_fd, char *str1, char *str2, char *str3,
                   uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS])
{
    int      rc;

    rc = fprint_puzzle_header(output_fd, str1, str2, str3);
    if (rc) {
        return (rc);
    }
    rc = fprint_puzzle_array(output_fd, board_array);

    return (rc);
}

/*
 * Print:
 * - the file header, constructed from the passed-in strings
 * - the puzzle values from an intermediate format array
 */
int
print_puzzle_file(char *str1, char *str2, char *str3,
                  uint4 board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS])
{
    int      rc;

    rc = fprint_puzzle_header(stdout, str1, str2, str3);
    if (rc) {
        return (rc);
    }
    rc = fprint_puzzle_array(stdout, board_array);

    return (rc);
}

