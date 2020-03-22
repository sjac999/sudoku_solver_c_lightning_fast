/*
 * Structures, defines and externs for the sudoku solver.
 *
 * Steve Jacobson, 01/05/06
 *
 * Copyright (c) 1996-2018 by Steven A. Jacobson, all rights reserved.
 */

#define TRUE                       1
#define FALSE                      0

typedef int                        bool;
typedef unsigned char              uint1;
typedef unsigned short             uint2;
typedef unsigned int               uint4;
typedef unsigned long long         uint8;

/*
 * The square root of the size of the puzzle.  The size of each dimension
 * must be the square of this value.
 */
#define SQRT_DIMENSION_SIZE        3
/* The number of cells in a row, column or square */
#define DIMENSION_SIZE             (SQRT_DIMENSION_SIZE * SQRT_DIMENSION_SIZE)
/* The number of cells in a row or column */
#define NUM_HORIZ_CELLS            (DIMENSION_SIZE)
#define NUM_VERT_CELLS             (DIMENSION_SIZE)
/* The number of squares in a row or column */
#define NUM_HORIZ_SQUARES          (SQRT_DIMENSION_SIZE)
#define NUM_VERT_SQUARES           (SQRT_DIMENSION_SIZE)
/* Width or height of a square in cells */
#define NUM_HORIZ_CELLS_IN_SQUARE  (SQRT_DIMENSION_SIZE)
#define NUM_VERT_CELLS_IN_SQUARE   (SQRT_DIMENSION_SIZE)
#define NUM_SIDE_CELLS_IN_SQUARE   (SQRT_DIMENSION_SIZE)
/* Total number of cells in the game */
#define TOT_NUM_CELLS              (DIMENSION_SIZE * DIMENSION_SIZE)

/* First dimension, the horizontal row */
#define ROW                        0
/* Second dimension, the vertical column */
#define COL                        1
/* Third dimension, the square */
#define SQUARE                     2

#define DEFAULT_MAX_NUM_RECURSION_LEVELS	4

#define INVALID_VALUE              ((uint4)-1)

/*
 * Defines for game->dprint.flags
 */
/* Misc. debug flags */
#define D_CRIT                     0x80000000
#define D_ERR                      0x40000000
#define D_INPUT                    0x00080000
#define D_PROCESS                  0x00040000
#define D_OUTPUT                   0x00020000
/* Solver algorithm debug flags */
#define D_CANTBE                   0x00000001
#define D_MUSTBE                   0x00000002
#define D_PAIR                     0x00000004
#define D_MTUP                     0x00000008
#define D_DIM                      0x00000010
#define D_SERP                     0x00000020
#define D_BFS                      0x00000100
#define D_DFS                      0x00000200
#define D_DFS2                     0x00000400
#define D_2MB                      0x00000800

/*
 * Defines for game->game_flags
 */
/* Recursive descent flags */
#define BREADTH_FIRST_1LR          0x00000001
#define REC_DESCENT                0x00000010
#define REC_DESCENT_ONLY           0x00000020
/* Random starting cell index flag */
#define RANDOM_START_INDEX         0x80000000
/* Non-standard, linear puzzle file input format */
#define LINEAR_FILE_FORMAT_IN      0x40000000
/* Non-standard, linear puzzle file output format */
#define LINEAR_FILE_FORMAT_OUT     0x20000000

/* Controls verbocity (not currently used) */
#define DS_SILENT                  0x00000001
#define DS_SUPER_SILENT            0x00000002


/*
 * depth_first_process_board() return status values
 */
enum recurse_return_status {
    rrs_success       = 0,
    rrs_system_error  = 1,
    rrs_depth_error   = 2,
    rrs_no_more_cells = 3,
    rrs_initial_error = 4,
};


//#ifdef REMOVED

/*
 * Structure containing debug print information
 */
typedef struct dprint_ {
    char     *log_filename;
    FILE     *log_fd;
    uint4    flags;
    uint4    silent_level;
} dprint_t;

//#endif  /* REMOVED */

/*
 * Structure containing stats collected during recursion.
 */
typedef struct recurse_stats_ {
    uint4     sum_num_row_changes;
    uint4     sum_num_col_changes;
    uint4     sum_num_square_changes;
    uint4     sum_num_tot_changes;
    uint4     iterations;
    uint4     sum_cells_tested;
    uint4     sum_values_tested;
    uint4     max_recursion_levels;
    uint4     tot_num_recursions;
    uint4     tot_num_backtracks;
    uint4     tot_num_not_sane;
} recurse_stats_t;

/*
 * Coordinate of one cell in a board
 */
typedef struct coord_ {
    char     row;
    char     col;
} coord_t;

/*
 * Coordinate of one cell in a board.
 * Used to hold state for board_find_next_undetermined_cell()
 */
typedef struct bfnuc_state_ {
    /* index of the first or next cell to examine in a board */
    uint4    state_index;
    /* number of cells examined so far in a board */
    uint4    state_count;
} bfnuc_state_t;

/*
 * Structure representing one cell of the board
 */
typedef struct cell_ {
    /* indices 0-8 represent values 1-9 */
    bool      values[DIMENSION_SIZE];
    /* number of values this cell might represent */
    uint4     num_possible_values;
    /* mark this cell for special attention */
    bool      mark_flag;
    /* row and column coordinates of this cell, zero indexed */
    uint4     row;
    uint4     col;
    /* square number of this cell, zero indexed, left-right, up-down */
    uint4     square;
} cell_t;

/*
 * Structure representing the entire board
 */
typedef struct board_ {
    /* Sudoku game cell array */
    cell_t          x_y_board[NUM_HORIZ_CELLS][NUM_VERT_CELLS];
    struct game_    *game;
    struct board_   *prev;
    struct board_   *next;
} board_t;

/*
 * Structure representing the state of the game
 */
typedef struct game_ {
    /*
     * malloced Sudoku game cell array and other state
     *   board_head:  points to the head of a linked list of board_t structs.
     *   board_curr:  points to the current board_t struct.
     */
    board_t          *board_head;
    board_t          *board_curr;
    /* malloced copy of the puzzle input file name */
    char             *in_filename;
    /* malloced copy of the puzzle output file name */
    char             *out_filename;
    /* Debug print information */
    dprint_t         dprint;
    uint4            game_flags;
    /* Limits depth of recursion in depth-first recursion function */
    uint4            max_num_recursion_levels;
    /* Initial cell index at which to start searching the board */
    uint4            initial_start_index;
    /* Stats structure passed for depth-first recursion */
    recurse_stats_t  *recurse_stats;
    /* Array of coordinates of each cell in a board */
    coord_t          coord[TOT_NUM_CELLS];
} game_t;


/*
 * Externs
 */
extern dprint_t *g_dprint;

