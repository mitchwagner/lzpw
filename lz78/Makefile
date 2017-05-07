.RECIPEPREFIX += 

GCC = gcc
GCCFLAGS = -std=c99 -W -Wall -O3 -ggdb3

PGCC = pgcc

OMPFLAGS = -fast -mp -Minfo -c99
MFLAGS = -acc -ta=multicore -Minfo=accel -c99
RFLAGS = -acc -ta=radeon -Minfo=accel -c99
NFLAGS = -acc -ta=nvidia -Minfo=accel -c99

lz78_omp: lz78.c
    $(PGCC) $(OMPFLAGS) -o $@ $^

lz78_acc: lz78.c
    $(PGCC) $(MFLAGS) -o $@ $^

lz78_nvidia: lz78.c
    $(PGCC) $(NFLAGS) -o $@ $^

lz78_amd: lz78.c
    $(PGCC) $(RFLAGS) -o $@ $^

lz78_test: lz78.c
    $(GCC) $(GCCFLAGS) -o $@ $^

all: $(OBJS) lz78_omp lz78_acc lz78_nvidia lz78_amd lz78_test

clean:
    rm -rf *.o *.out
