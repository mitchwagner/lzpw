#ifndef LZ78_H
#define LZ78_H

/**
 * This file exposes an interface for bitwise encryption via the LZ78
 * algorithm.  The dictionary is implemented naively, via an array of
 * characters.
 *
 * In the LZ78 algorithm, each entry in the dictionary consists of two parts:
 * an offset into the dictionary potentially referencing a previous entry,
 * and a bit to append to that entry. In this way, longer and longer words
 * can be built up by the self-referential nature of the dictionary entries.
 *
 * In this implementation, every entry in the dictionary is some multiple n of
 * bytes (1, 2, etc.) The last (n * 8) - 1 bits encode an offset into the
 * dictionary. The first bit is the bit to append.
 */

 // TODO: It would be good to store a header file describing information about
 // the encryption. Of course, at that point, following standard ZIP format
 // might be more beneficial

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * Simple dictionary, implemented as a list, for storing previously-seen
 * patterns. 
 * TODO: Might be better to encode ref_size in the dictioanry struct itself
 */
typedef struct _dict {
    uint64_t size; // Number of entries in the dictionary
    char* dict;    // Array in which to store previously-seen patterns
} dictionary;

typedef struct _header {
    char ref_size;    // Size of each dictionary reference
    int num_threads;  // Number of threads in the file
    long* split_locs; // Indices the file is split on
} header;

/**
 * Mallocs appropriate amount of memory for a dictionary's dict array, based
 * on the largest reference size possible in the dictionary.
 * TODO: This should be refactored
 *
 * @param ref_size The size, in bits, of each dictionary reference
 * @return The malloc'd memory
 */
char* init_lookup(int ref_size);

/**
 * Look up an entry in a dictionary.
 *
 * @param dict The dictionary involved in lookup
 * @param entry Pointer to the entry to find in the dictionary
 * @param ref If lookup is successful, store resulting entry at location 
 *     indicated by this pointer
 * @param ref_size The size, in bits, of each dictionary reference.
 *
 * @return true if lookup is successful, false if otherwise
 */
bool lookup(dictionary* dict, char* entry, uint64_t* ref, int ref_size);

/**
 * Store an entry in a dictionary.
 *
 * @param dict The dictionary in which to store an entry
 * @param entry The entry to store in the dictionary
 * @param ref The location to store the entry to
 * @ref_size The size, in bits, of each dictionary reference
 */
bool store(dictionary* dict, char* entry, uint64_t ref, int ref_size);

/**
 * This is a utility function for getting the next bit from a file. It
 * incrementally gets a byte from the file, keeping track of the offset into
 * the byte. When the byte is used up, it gets the next one.
 *
 * @param c The location to store the character
 * @param itr The offset into the current byte
 * @param in The file stream being read
 */
static int get_next_bit(char* c, uint64_t itr, FILE* in); 

/**
 * Takes a number of bits, and returns the number of bytes necessary to 
 * hold that number of bits.
 *
 * @param ref_size The size, in bits, of each dictionary reference
 * TODO: The fact that I have to keep re-typing the above as a parameter
 *       is a strong indication of a code smell
 */
int get_num_bytes(int ref_size);

/**
 * Takes a maximum size of a reference, in bits, and returns the number of 
 * bytes the dictionary will use to store both the reference and an additional
 * bit.
 * @param ref_size The maximum size of a reference, in bits
 * @return The number of bytes needed to hold the reference + 1 bit
 */
int get_num_bytes(int ref_size);

/**
 * Encodes a file with LZ78 encryption for compression
 * 
 * @param ref_size The size, in bits, of each dictionary reference
 * @param infile The file to compress
 * @param outfile The name of the output file
 */
int encode(int ref_size, const char * const infile, const char * const outfile);

/**
 * TODO: Document!
 */
void parallel_encode(int num_threads, int ref_size, const char * const infile,
    const char * const outfile);

/**
 * Decodes a file encrypted by this LZ78 implementation
 * @param ref_size The size, in bits, of each dictioanry reference 
 * @param infile The file to decode
 * @param outfile The name of the output file
 */
int decode(int ref_size, const char * const infile, const char * const outfile);

/**
 * TODO: Document!
 */
int parallel_decode(const char * const infile, const char * const outfile);

/**
 * Utility function to combine a bit and a dictionary reference into a single
 * dictionary entry.
 *
 * @param entry The location to store the combined entry
 * @param ref_size The size, in bits, of each dictionary reference
 * @param ref The dictionary reference
 * @param bit The bit 
 */
static int create_entry(char* entry, int ref_size, uint64_t ref, int bit);

static long long get_file_size(const char * const file);

int encode_help(int ref_size, long long start, long long end, FILE* in, FILE* out);

int decode_help(int ref_size, long long start, long long end, FILE* in, FILE* out);

int encoding_merge(FILE** files, int num_files, char* outfile, int dict_size);

// Intended to combine multiple files into one
int merge_files(FILE** files, int num_files, FILE* outfile, int dict_size);

int make_header(int dict_size, int num_threads, long long* split_locs, FILE* out);

header* read_header(FILE* in);

// Intended to remove unnecessary files
int clean_up();

#endif
