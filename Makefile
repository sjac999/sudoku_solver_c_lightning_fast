#
# Makefile for sudoku solver
#

# gcc for Linux, cc for FreeBSD
CC	= cc
# Runs about twice as fast with -O4
CFLAGS	= -Wall -O0 -g

all:
	$(CC) $(CFLAGS) -c sudoku.c
	$(CC) $(CFLAGS) -c sudoku_io.c
	$(CC) $(CFLAGS) -c sudoku_printd.c
	$(CC) $(CFLAGS) -c sudoku_io_test.c
	$(CC) $(CFLAGS) -o sudoku sudoku.o sudoku_io.o sudoku_printd.o
	$(CC) $(CFLAGS) -o sudoku_io_test sudoku_io_test.o sudoku_io.o sudoku_printd.o

clean:
	rm sudoku sudoku_io_test *.o

