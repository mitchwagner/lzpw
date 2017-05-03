.RECIPEPREFIX += 

# Okay, need to make it normally with gcc
# need to make it with omp
# need to make it with acc for both platforms

GCC = gcc
GCCFLAGS = -std=c99 -W -Wall -Werror -O3 -ggdb3

PGCC = pgcc

OMPFLAGS = -fast -mp -Minfo
MFLAGS = -acc -ta=multicore -Minfo=accel
NFLAGS = -acc -ta=radeon -Minfo=accel
RFLAGS = -acc -ta=nvidia -Minfo=accel

OBJS = main.o

main.o: main.c main.h

lz78_omp: $(OBJS)
    $(PGCC) $(OMPFLAGS) -o lz78_omp $(OBJS);

lz78_acc: $(OBJS)
    $(PGCC) $(OMPFLAGS) -o lz78_acc $(OBJS);

lz78_nvidia: $(OBJS)
    $(PGCC) $(NFLAGS) -o lz78_nvidia $(OBJS);

lz78_amd: $(OBJS)
    $(PGCC) $(RFLAGS) -o lz78_amd $(OBJS);

lz78_test: $(OBJS)
    $(GCC) $(GCCFLAGS) -o lz78_test $(OBJS);

all: $(OBJS) lz78_omp lz78_acc lz78_nvidia lz78_amd lz78_test

clean:
    rm -rf *.o *.out
