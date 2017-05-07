#ifndef LZ78_H
#define LZ78_H

/**
 * This file exposes an interface for bitwise encryption via the LZ78
 * algorithm.  The dictionary is implemented naively, via an array of
 * characters. Currently, only little endian machines are supported.
 *
 * In the LZ78 algorithm, each entry in the dictionary consists of two parts:
 * an offset into the dictionary potentially referencing a previous entry,
 * and a bit to append to that entry. In this way, longer and longer words
 * can be built up by the self-referential nature of the dictionary entries.
 *
 * In this implementation, every entry in the dictionary is some multiple n of
 * bytes (1, 2, etc.) The last (n * 8) - 1 bits encode an offset into the
 * dictionary. The first bit is the bit to append.
 *
 * Parallel encryption is implemented via breaking the input file into 
 * separate pieces and running the compression algorithm in parallel
 * on each piece, merging the pieces at the end.
 *
 * In order to facilitate decompression, a header is appended to the
 * beginning of every compressed file:
 *
 * [dict_size][num_threads][split_locations (num_threads of them)]
 *
 * dict_size: encodes how large the dictionary used to compress the
 *     data was 
 *
 * num_threads: the number of partitions used to split the data
 *
 * split_locations: an array denoting the position in the
 *     compressed file that are the beginning of merged files
 *     that need to be decompressed individually
 *
 * Author: Mitch Wagner
 * Date:   May 8, 2017
 */

// TODO: Describe the compressed header here
// TODO: Only supports little-endian machines
// TODO: So many of these should be static

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * Simple dictionary, implemented as a list, for storing previously-seen
 * patterns. The dictionary is currently implemented such that each entry
 * in the dictionary consists of some number of bytes. The first bit of that
 * entry is the bit to append to the bit sequence referenced by the rest of 
 * the bits when shifted to the right. If the reference is 0, the bit is the 
 * last bit in the sequence.
 *
 * Example:
 * 0: LEFT EMPTY
 * 1: 0000 0001 Encodes 1
 * 2: 0000 0010 Encodes 10
 * 3: 0000 0000 Encodes 0
 * 4: 0000 0101 Encodes 101
 *
 * TODO: When the dictionary fills up, no new entries are added. Might
 * be better to just start over to exploit symmetry through locality 
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
 * @param ref If lookup is successful, store resulting entry at location *     indicated by this pointer
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
 * Compresses a file using LZ78 
 * 
 * @param ref_size The size, in bits, of each dictionary reference
 * @param infile The file to compress
 * @param outfile The name of the output file
 * @return TODO
 */
int encode(int ref_size, const char * const infile, const char * const outfile);

/**
 * Compress a file in parallel using LZ78
 *
 * @param num_threads The number of threads to use while compressing
 * @param ref_size The size of the dictionary to use for compression
 * @param infile The file to compress
 * @param outfile The name of the output file
 */
void parallel_encode(int num_threads, int ref_size, const char * const infile,
    const char * const outfile);

/**
 * Decodes a file encrypted by this LZ78 implementation
 *
 * @param ref_size The size, in bits, of each dictioanry reference 
 * @param infile The file to decompress 
 * @param outfile The name of the output file
 * @return TODO
 */
int decode(int ref_size, const char * const infile, const char * const outfile);

/**
 * Decompresses a file, compressed by this program using LZ78, in parallel
 *
 * @param num_threads The number of threads to use in decompression
 * @param infile The file to decompress
 * @param outfile The name of the output file
 * @return TODO
 */
int parallel_decode(int num_threads, const char * const infile, const char * const outfile);

/**
 * Utility function to combine a bit and a dictionary reference into a single
 * dictionary entry.
 *
 * @param entry The location to store the combined entry
 * @param ref_size The size, in bits, of each dictionary reference
 * @param ref The dictionary reference
 * @param bit The bit 
 * @return TODO
 */
static int create_entry(char* entry, int ref_size, uint64_t ref, int bit);

/**
 * Utility function for getting the length of a given file
 *
 * @param file The name of the file whose length to get
 * @return The length of the file
 */
static long long get_file_size(const char * const file);

/**
 * Called by each thread during parallel compression 
 * 
 * @param ref_size The size, in bits, of each dictionary reference
 * @param start The location in the file the thread should begin compression at
 * @param end The location in the file the thread should cease compression at
 * @param in The file to compress
 * @param out Temporary file the thread should write its work to. Each thread's
 *     temporary file will ultimately be merged to create the output file.
 * @return TODO
 */
int encode_help(int ref_size, long long start, long long end, FILE* in, FILE* out);

/**
 * Called by each thread during parallel decompression, or by a single 
 * thread as it decompresses each part of a parallel-compressed file.
 *
 * @param ref_size The size, in bits, of each dictionary reference during 
 *     compression
 * @param start The location in the file the thread should begin 
 *     decompression at
 * @param end The location in the file the thead should cease decompression at
 * @param in The file to be decompressed
 * @param out Temporary file the thread should write its work to. Each thread's
 *     temporary file will ultimately be merged to create the output file.
 * @return TODO
 */
int decode_help(int ref_size, long long start, long long end, FILE* in, FILE* out);

/**
 * Merges a number of files together, adding a header to describe the 
 * compression conditions (needed to facilitate decompression).
 *
 * @param files The files to merge together
 * @param num_files The number of files to be merged together
 * @param outfile The name of the resulting output file
 * @param dict_size The size of the dictionary entries used for compression
 * @return TODO
 */
int encoding_merge(FILE** files, int num_files, char* outfile, int dict_size);

/**
 * Merges an array of files into a single file
 *
 * @param files The files to merge together
 * @param num_files The number of files to merge together
 * @param outfile The name of the resulting merged file 
 * @return TODO
 */
int merge_files(FILE** files, int num_files, FILE* outfile);

/**
 * Adds this implementation's LZ78 header to the beginning of a compressed file.
 *
 * @param dict_size The size of the dictionary entries used for compression
 * @param num_threads The number of partitions encoded in parallel and merged
 * @param split_locs The locations of each partition in the merged file
 * @param out The output file to add the header to
 * @return TODO
 */
int make_header(int dict_size, int num_threads, long long* split_locs, FILE* out);

/**
 * Reads the header at the beginning of a compressed file,
 * and packs the information into a header object
 *
 * @param in A file compressed by this LZ78 implementation
 * @return Pointer to a header object
 */
header* read_header(FILE* in);

#endif
