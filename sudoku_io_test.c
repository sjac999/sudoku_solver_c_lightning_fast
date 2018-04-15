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
    /* Debug print information */
    dprint_t  *dprint;
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
     * range checked; zero represents '.' from the input file.
     */
    rc = read_puzzle_file(pi->input_fd, pi->input_board_array);
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
    printf("\nusage: %s [-?hsV] [-i <ifile>] [-o <ofile>] [-d <debug flags>]\n",
        argv[0]);
    printf("   -?h:		print help and exit\n");
    printf("   -d:		set debug flags\n");
    printf("   -i:		specify input filename\n");
    printf("   -o:		specify output filename\n");
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
    char           *in_fname = "----";
    char           *out_fname = "----";
    char           *endptr;
    uint4          debug = 0;
    uint4          levels = 0;
    int            opt;
    int            rc;

#ifdef REMOVED
    pi->input_fd = NULL;
    pi->output_fd = NULL;
    pi->in_filename = NULL;
    pi->out_filename = NULL;
#endif

    /*
     * Note:  Can hack puzzle name into program for testing like the following:
     */
    //pi->in_filename = "./puzzles/d6.txt";
    //pi->out_filename = "./p2/d6m.txt";

    /*
     * Command line options handling
     */
    while ((opt = getopt(argc, argv, "?d:f:hi:o:s:V")) != EOF) {
        switch (opt) {
        case 'd':
            debug = strtoul(optarg, &endptr, 0);
            if (errno != ERANGE && *endptr == '\0') {
                pi->dprint->flags = debug;
            } else {
                printf("Invalid debug flags 0x%x\n", debug);
                //free(filename);
                return (-1);
            }
            break;
        case 'i':
            pi->in_filename = strdup(optarg);
            break;
        case 'o':
            pi->out_filename = strdup(optarg);
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

    //in_fname = "./puzzles/l4-30.txt";
    //out_fname = "./p2/l4-30.txt";

    if (!pi->in_filename) {
        pi->in_filename = strdup(in_fname);
    }
    if (!pi->out_filename) {
        pi->out_filename = strdup(out_fname);
    }

    printd(D_INPUT, "input filename:  %s\n", pi->in_filename);
    printd(D_INPUT, "output filename:  %s\n", pi->out_filename);

    pi->input_fd = fopen(pi->in_filename, "r");
    if (!pi->input_fd) {
        printd(D_CRIT, "\n*** File \"%s\" does not exist!\n", pi->in_filename);
        return (-1);
    }
    pi->output_fd = fopen(pi->out_filename, "wx");
    if (!pi->output_fd) {
        printd(D_CRIT, "\n*** File \"%s\" already exists!\n", pi->out_filename);
        return (-2);
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
 * Processes the input array values, then copies the result to the
 * output array.
 */
int
process_array_values(prog_info_t *pi)
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
 * Transfers the output array to the output file.
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

    rc = fprint_puzzle_file(pi->output_fd,
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

    rc = process_array_values(pi);
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

