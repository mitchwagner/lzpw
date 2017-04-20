#ifndef LZ78_H
#define LZ78_H

#include <stdint.h>
#include <stdbool.h>


typedef struct _dict {
    uint64_t size;
    char* dict;
} dictionary;

char* init_lookup(int ref_size);
bool lookup(dictionary* dict, char* entry, uint64_t* ref, int ref_size);
bool store(dictionary* dict, char* entry, uint64_t ref, int ref_size);

int get_next_bit(char* c, uint64_t itr, FILE* in); int get_num_bytes(int ref_size);

/**
 * Takes a maximum size of a reference, in bits, and returns the number of 
 * bytes the dictionary will use to store both the reference and an additional
 * bit.
 * @param ref_size The maximum size of a reference, in bits
 * @return The number of bytes needed to hold the reference + 1 bit
 */
int get_num_bytes(int ref_size);

int encode(int ref_size, const char * const infile, const char * const outfile);
int decode(int ref_size, const char * const infile, const char * const outfile);

int create_entry(char* entry, int ref_size, uint64_t ref, int bit);

#endif
