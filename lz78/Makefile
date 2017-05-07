.RECIPEPREFIX += 

GCC = gcc
GCCFLAGS = -std=c99 -W -Wall -O3 -ggdb3

PGCC = pgcc

OMPFLAGS = -fast -mp -Minfo -c99
MFLAGS = -acc -ta=multicore -Minfo=accel -c99
RFLAGS = -acc -ta=radeon -Minfo=accel -c99
NFLAGS = -acc -ta=nvidia -Minfo=accel -c99

# OBJS = main.o

# main.o: main.c main.h
#     $(PGCC) $(OMPFLAGS) -c99 -c main.c

lz78_omp: main.c
    $(PGCC) $(OMPFLAGS) -o $@ $^

lz78_acc: main.c
    $(PGCC) $(MFLAGS) -o $@ $^

lz78_nvidia: main.c
    $(PGCC) $(NFLAGS) -o $@ $^

lz78_amd: main.c
    $(PGCC) $(RFLAGS) -o $@ $^

lz78_test: main.c
    $(GCC) $(GCCFLAGS) -o $@ $^

all: $(OBJS) lz78_omp lz78_acc lz78_nvidia lz78_amd lz78_test

clean:
    rm -rf *.o *.out
