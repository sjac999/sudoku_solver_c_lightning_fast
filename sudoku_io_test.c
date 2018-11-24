/*
 * Test file input and output for C language Sudoku solver
 *
 * Steve Jacobson, 04/06/06
 *
 * Copyright (c) 1996-2018 by Steven A. Jacobson, all rights reserved.
 */

static const char rcsid[]="sudoku_io_test.c,v 1.1 2018/04/09 07:05:11 stevej Exp";

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "sudoku.h"
#include "sudoku_io.h"
#include "sudoku_printd.h"


/*
 * Structure containing program information.
 */
typedef struct prog_info_ {
    FILE      *input_fd;
    FILE      *output_fd;
    /* malloced copies of the file name strings */
    char      *in_filename;
    char      *out_filename;
    /* Input file format */
    enum input_file_format  in_file_format;
    /* Output file format */
    enum output_file_format  out_file_format;
    /* Debug print information */
    dprint_t  *dprint;
    /* Process board array values if TRUE */
    bool      process_array_values;
    uint4     input_board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS];
    uint4     output_board_array[NUM_VERT_CELLS][NUM_HORIZ_CELLS];
} prog_info_t;

/*
 * Globals
 */
dprint_t   *g_dprint = NULL;


/*
 * Initialize program_data structures.
 */
prog_info_t *
program_init(void)
{
    prog_info_t   *pi;

    /*
     * Allocate the program info structure
     */
    pi = malloc(sizeof(prog_info_t));
    if (!pi) {
        return (NULL);
    }

    /*
     * Initialize debug printing
     */
    g_dprint = malloc(sizeof(dprint_t));
    if (!g_dprint) {
	free(pi);
        return (NULL);
    }
    printd_init(g_dprint, NULL);
    g_dprint->silent_level = 1;

    pi->dprint = g_dprint;
    pi->input_fd = NULL;
    pi->output_fd = NULL;
    pi->in_filename = NULL;
    pi->out_filename = NULL;
    pi->process_array_values = FALSE;

    return (pi);
}

/*
 * Clean up the program data structures.
 */
void
program_cleanup(prog_info_t *pi)
{
    if (pi->input_fd) {
        fclose(pi->input_fd);
    }
    if (pi->output_fd) {
        fclose(pi->output_fd);
    }
    if (pi->in_filename) {
        free(pi->in_filename);
    }
    if (pi->out_filename) {
        free(pi->out_filename);
    }

    printd_cleanup(pi->dprint);
    free(pi->dprint);
    free(pi);
}

/*
 * Reads the input file and populates the initial board in the game.
 */
int
input_puzzle_file(prog_info_t *pi)
{
    int      rc;

    /*
     * Reads the input file into the intermediate format.  Values are
     * range checked; zero represents '.' from the standard input file.
     */
    rc = read_puzzle_file(pi->input_fd, pi->in_file_format,
      pi->input_board_array);
    if (rc) {
        return (rc);
    }

    if (0 == pi->dprint->silent_level) {
        print_puzzle_array(pi->input_board_array);
    }

    // Fixme
    // Fixme:  Debugging code only.
    // Fixme
    //print_puzzle_file("Source:  ", "Date:    ", "Level:   ",
    //    pi->input_board_array);

    return (0);
}

/*
 * Print the usage string
 */
void
usage(int argc, char **argv)
{
    printf("\nusage: %s [-?hsV] [-iI <ifile>] [-oO <ofile>] [-d <debug flags>]\n",
        argv[0]);
    printf("   -?h:		print help and exit\n");
    printf("   -d:		set debug flags\n");
    printf("   -i:		specify input filename (standard format)\n");
    printf("   -I:		specify input filename (linear format)\n");
    printf("   -o:		specify output filename (standard format)\n");
    printf("   -O:		specify output filename (linear format)\n");
    printf("   -p:		process input; don't just copy\n");
    printf("   -s:		silent mode level\n");
    printf("   -V:		print version string and exit\n");
}

/*
 * Processes command line arguments.
 * Opens input and output files.
 * Loads values from input file to input array.
 */
int
input_values(int argc, char **argv, prog_info_t *pi)
{
    char           *endptr;
    uint4          debug = 0;
    uint4          levels = 0;
    int            opt;
    int            rc;

    /*
     * Command line options handling
     */
    while ((opt = getopt(argc, argv, "?d:f:hi:I:o:O:ps:V")) != EOF) {
        switch (opt) {
        case 'd':
            debug = strtoul(optarg, &endptr, 0);
            if (errno != ERANGE && *endptr == '\0') {
                pi->dprint->flags = debug;
            } else {
                printf("Invalid debug flags 0x%x\n", debug);
                return (-1);
            }
            break;
        case 'i':
            pi->in_filename    = strdup(optarg);
            pi->in_file_format = input_standard;
            break;
        case 'I':
            pi->in_filename    = strdup(optarg);
            pi->in_file_format = input_linear;
            break;
        case 'o':
            pi->out_filename    = strdup(optarg);
            pi->out_file_format = output_standard;
            break;
        case 'O':
            pi->out_filename    = strdup(optarg);
            pi->out_file_format = output_linear;
            break;
        case 'p':
            /* process input values if TRUE, else just copy to output */
            pi->process_array_values = TRUE;
            break;
        case 's':
            levels = strtoul(optarg, &endptr, 0);
            if (errno != ERANGE && *endptr == '\0') {
                pi->dprint->silent_level = levels;
            } else {
                printf("Invalid silent level %d\n", levels);
                return (-1);
            }
            break;
        case 'V':
            printf("Version:  %s\n", rcsid);
            return (1);
            break;
        case 'h':
        case '?':
            usage(argc, argv);
            return (1);
            break;
        }
    }

    if (!pi->in_filename) {
        printd(D_CRIT, "\n*** Input filename not specified!\n");
        return (-1);
    }
    if (!pi->out_filename) {
        printd(D_CRIT, "\n*** Output filename not specified!\n");
        return (-2);
    }

    printd(D_INPUT, "input filename:  %s\n", pi->in_filename);
    printd(D_INPUT, "output filename:  %s\n", pi->out_filename);

    pi->input_fd = fopen(pi->in_filename, "r");
    if (!pi->input_fd) {
        printd(D_CRIT, "\n*** File \"%s\" does not exist!\n", pi->in_filename);
        return (-3);
    }
    pi->output_fd = fopen(pi->out_filename, "wx");
    if (!pi->output_fd) {
        printd(D_CRIT, "\n*** File \"%s\" already exists!\n", pi->out_filename);
        return (-4);
    }

    /*
     * Reads the input file and populate the input array.
     */
    rc = input_puzzle_file(pi);
    if (rc) {
        return (rc);
    }

    return (0);
}

/*
 * Copies the input array values to the output array.
 */
int
copy_array_values(prog_info_t *pi)
{
    uint4    row;
    uint4    col;

    /*
     * For test purposes, copy input array to output array.
     */
    for (row=0; row < NUM_VERT_CELLS; row++) {
        for (col=0; col < NUM_HORIZ_CELLS; col++) {
            pi->output_board_array[row][col] = pi->input_board_array[row][col];
        }
    }

    return (0);
}

/*
 * Processes the input array values, then copies the result to the
 * output array.  Add any code to process the values from the input
 * puzzle here.
 */
int
process_array_values(prog_info_t *pi)
{
    int      rc;

    rc = copy_array_values(pi);

    return (rc);
}

/*
 * Writes the output array to the output file.
 * Prepends the array data with a generic header.
 */
int
output_array_values(prog_info_t *pi)
{
    int            rc;

    if (0 == pi->dprint->silent_level) {
        print_puzzle_file("Source:  ", "Date:    ", "Level:   ",
            pi->output_board_array);
    }

    rc = fprint_puzzle_file(pi->output_fd, pi->out_file_format,
        "Source:  ", "Date:    ", "Level:   ",
        pi->output_board_array);

    return (rc);
}

/*
 * main()
 */
int
main(int argc, char **argv)
{
    prog_info_t    *pi;
    int            rc;

    pi = program_init();

    rc = input_values(argc, argv, pi);
    if (rc) {
        if (0 > rc) {
            printf("\n*** Input error %d!  Exiting!\n", rc);
        }
        program_cleanup(pi);
        exit(15);
    }

    /* process input values if TRUE, else copy to output unchanged */
    if (pi->process_array_values) {
        rc = process_array_values(pi);
    } else {
        rc = copy_array_values(pi);
    }
    if (rc) {
        if (0 > rc) {
            printf("\n*** Processing error %d!  Exiting!\n", rc);
        }
        program_cleanup(pi);
        exit(16);
    }

    rc = output_array_values(pi);
    if (rc) {
        if (0 > rc) {
            printf("\n*** Output error %d!  Exiting!\n", rc);
        }
        program_cleanup(pi);
        exit(17);
    }

    program_cleanup(pi);

    exit(0);
}

