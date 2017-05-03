.RECIPEPREFIX += 

CC = gcc
CFLAGS = -std=c99 -W -Wall -Werror -O3 -ggdb3

OBJS = main.o

main.o: main.c main.h

all: $(OBJS)
    $(CC) $(CFLAGS) -o lz78 $(OBJS);

clean:
    rm -rf *.o *.out
