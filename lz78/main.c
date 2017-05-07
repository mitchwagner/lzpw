// Should make sure that I have proper return values everywhere
// Store documentation in HEADER files.
// Need to make sure that every function in here is in the header file

// There is a difference between bit-level and byte-level
// encoding. By using bytes, I'm literally just passing the buck and saying
// that I am hoping that specific combinations of bytes appear frequently,
// as opposed to saying that I hope specific combinations of bits appear
// frequently...
// But are the odds that a byte follows a byte as good as a bit following a
// a bit? I think not. There are SO MANY combinations different bytes! 256, to
// be precise, whereas there are only 2 combinations of different bits...
// 

// Okay, so, here is the deal. I could easily make it operate on bytes.
// Dictionary:
// [byte][variable length references
// [byte][variable length references
// [byte][variable length references
// I could also make it operate on bits:
// Dictionry:
// [reference][1]
// [reference][1] // Where the last bit of each entry is the new bit
// Now, theoretically, I could make that work with any size reference.
// HOWEVER, because the fundamental unit is a BYTE and not a bit,
// the second scheme becomes MUCH MORE CONVENIENT to access if the reference
// size is 7 bytes, or 15 bytes, or 23, or 31, etc., because otherwise,
// I have to do math to access the right bytes. This way, I can access grab
// a constant number n bytes, throw them away, and grab the next pair. The
// other scheme is possible, but more difficult to implement.

// Okay, so that raises another issue...
// How do we make sure that everything works out correctly w/r.t. GPU
// writing to buffers? I have to transfer data and allocate enough space on the
// GPU to write to these files...or else, I just have to let the GPUs write
// to the files, right? That would seem to be the most prudent option, because
// that is how I do things now: buffered file IO. If I wanted to do 
// something where I was writing to buffers instead of files, I would have to
// figure out how that would work serially before I looked parallel.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <omp.h>
#include "main.h"

// TODO: Add version info that keeps track of omp, serial, etc. 
void print_usage(char* prog_name) {
    printf("LZ78 Encryption Tool:\n");
    printf("Usage: %s op infile outfile [num_threads] [dict_size]\n" , prog_name);
    printf("    op:          \"compress\" or \"decompress\"\n");
    printf("    infile:      file to compress\n");
    printf("    outfile:     file to write result to\n");
    printf("    num_threads: number of threads to use (1-64) \n");
    printf("    dict_size:   size, in bytes, of dictionary entry (1 or 2)\n");
}

int main(int argc, char** argv){

    if (argc != 4 && argc != 5 && argc != 6) {
        print_usage(argv[0]);
        exit(1);
    }

    char* infile = argv[2];
    char* outfile = argv[3];
    int dict_size = 15;

    int num_threads = 1;

    if (argc == 5) {
        num_threads = strtol(argv[4], NULL, 10);
    }

    if (argc == 6) {
        int size = strtol(argv[5], NULL, 10);

        if (size == 1) {
            dict_size = 7;
        }
        else if (size == 2) {
            dict_size = 15;
        }
        else {
            print_usage(argv[0]);
            exit(1);
        }
    }

    if (strcmp(argv[1],"compress") == 0) {
        // TODO: Error checking 
        // encode(dict_size, infile, temp); 
        parallel_encode(num_threads, dict_size, infile, outfile);
    }
    else if (strcmp(argv[1], "decompress") == 0) {
        // TODO: Error checking
        // decode(dict_size, temp, outfile);
        // TODO: Add numthreads to this
        parallel_decode(num_threads, infile, outfile);
    }
    else {
        print_usage(argv[0]);
        exit(1);
    }

    return 0;
}

static int get_next_bit(char* c, uint64_t itr, FILE* in){
    if (itr == 0) {
        *c = fgetc(in);
        if (feof(in)){
            return -1;
        }
    }
    int val = *c & 1;
    *c = *c >> 1;
    return val;
}

/**
 * Operation proceeds as follows:
 * 1) Get the size of the file to be compressed, in bytes
 * 2) Create an array of pointers to the file to be read and the files 
 *    to be written
 * 3) Partition the infile by advancing pointers by standard increment
 *    per thread
 * 4) Have each file write to their respective outfile, compressing
 *    their respective parts of the infile
 * 5) Merge the outfiles, adding a header for information
 */
void parallel_encode(int num_threads, int ref_size, const char * const infile,
    const char * const outfile){

    long long fsize = get_file_size(infile);

    FILE** infiles  = malloc(num_threads * sizeof(FILE*));
    FILE** outfiles = malloc(num_threads * sizeof(FILE*));

    // Each thread should have about this much work to do
    // TODO: Make sure this is safe for extremely small files (it probably
    // is not). Since I divide fsize by num_threads, increment could become
    // 0. Not good.
 
    // TODO: Should be be an unsigned long? How big can a file be, anyway?
    // In reality, this could would have to be a lot safer...
    long long increment = fsize / num_threads;
    printf("Increment: %llu\n", increment);

    // TODO: num_threads is not an accurate description of this variable.
    // In reality, it is closer to num_partitions than anything else
    #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < num_threads; i++) {
        printf("Thread rank: %d\n", omp_get_thread_num());
        printf("Max threads: %d\n", omp_get_thread_limit());
        infiles[i] = fopen(infile, "rb");
        fseek(infiles[i], i * increment, SEEK_SET);

        outfiles[i] = tmpfile();

        long long end;
        if (i == num_threads - 1){
            end = fsize;	
        }
        else {
            end = (i + 1) * increment;
        }

        encode_help(ref_size, i * increment, end, infiles[i], outfiles[i]);

        // TODO: I might not need to do this here, might just slow
        // things down.
        fflush(outfiles[i]);
    } 

    // TODO: Honestly, I do this enough that I should make
    // look at making it a method somehow
    char outname[strlen(outfile) + 10];
    memset(outname, 0, strlen(outfile) + 10);
    memcpy(outname, outfile, strlen(outfile));
    encoding_merge(outfiles, num_threads, outname, ref_size);

    for (int i = 0; i < num_threads; i++) {
        fclose(infiles[i]);
	fclose(outfiles[i]);
    }

    free(infiles);
    free(outfiles);
}

// TODO: This only allows little endian
int make_header(int dict_size, int num_threads, long long* split_locs, FILE* out) {
    char sz = (char) dict_size;

    fwrite(&sz, sizeof(char), 1, out);
    fwrite(&num_threads, sizeof(int), 1, out);
    for (int i = 0; i < num_threads; i++) {
        fwrite(&split_locs[i], sizeof(long long), 1, out);
    }

    return 0;
}

// TODO: Endianness troubles, only supporting little
header* read_header(FILE* in){
    header* h = malloc(sizeof(header));
    fread(&h->ref_size, sizeof(char), 1, in);
    fread(&h->num_threads, sizeof(int), 1, in);
    printf("Bits: %d\n", h->ref_size);
    printf("Threads: %d\n", h->num_threads);
    h->split_locs = malloc(sizeof(long long) * h->num_threads);

    for (int i = 0; i < h->num_threads; i++) {
        fread(&(h->split_locs[i]), sizeof(long long), 1, in);
    }
    return h;
}

int get_header_length(header* h) {
    return 1 + 4 + h->num_threads * sizeof(long);
}

int encoding_merge(FILE** files, int num_files, char* outfile, int 
    dict_size){
    
    FILE* out = fopen(outfile, "wb");
    long long split_locs[num_files];

    for (int i = 0; i < num_files; i++) {
        if (i == 0) {
            split_locs[i] = 1 + 4 + num_files * sizeof(long long);
        }
        else {
            fseek(files[i - 1], 0, SEEK_END);
            long long len = ftell(files[i - 1]);
            printf("len from fseek: %llu\n", len);
            split_locs[i] = split_locs[i - 1] + len; 
        }
        printf("Starting spot is: %llu\n", split_locs[i]);
    }

    make_header(dict_size, num_files, split_locs, out);

    merge_files(files, num_files, out);
    
    fclose(out);

    // TODO: Actual error checking
    return 0;
}

// Want to make sure that I can re-use this code in the decoding phase
int merge_files(FILE** files, int num_files, FILE* outfile) {

    int ch;
    for (int i = 0; i < num_files; i++) {
        //printf("File length: %llu\n", get_file_size(files[i]));
        fseek(files[i], 0, SEEK_SET);
        while((ch = fgetc(files[i]) ) != EOF ){
            fputc((char)(ch),outfile);
        }
    }
    printf("Done merging!\n\n");
   
    // TODO: Actual error checking
    return 0;
}


// TODO: I should really abstract the bulk of the functionality here and 
// in the serial encode method into a single, callable function, probably
// that takes file pointers and the rest of the parameters here. ACTUALLY,
// on that note, maybe I should abstract the serial to just call this
// function!
int encode_help(int ref_size, long long start, long long end, FILE* in, FILE* out) {
    char c = 0;

    short itr = 0; 
    int bit = 0;
    uint64_t ref = 0;
    bool present = false;

    // Char array big enough to hold the reference
    char entry[get_num_bytes(ref_size)];

    dictionary dict; 
    dict.dict = init_lookup(ref_size);

    // This might be a bug in the distant future.
    // This is to avoid conflicts that point things to the 0th index, and
    // lets us reserve 0 to indicate no preceding character.
    dict.size = 1;

    // Number of bits this will read
    long long bits_to_get = (end - start) * 8;
    long long count = 0;

    while ((bit = get_next_bit(&c, itr, in)) != -1 && count < bits_to_get) {
        count++;

        //printf("%d", bit);
        itr = (itr + 1) % 8;

        create_entry(entry, ref_size, ref, bit);

        // Get updated index if it exists in the table
        present = lookup(&dict, entry, &ref, ref_size);

        //printf(" was found: %s", present ? "true\n" : "false\n");

        if (!present) {
            store(&dict, entry, dict.size, ref_size);
            // This shouldn't be done here...
            fwrite(entry, 1, get_num_bytes(ref_size), out);
            ref = 0;
        }
        else if (present){
            // printf("ref is %" PRIu64 "\n" ,  ref);
            // Ref has been updated. Get next character and continue process
        }
    }
    if (ref != 0) {
        fwrite(entry, 1, get_num_bytes(ref_size), out);
    }

    free(dict.dict);
    return 0;
}

// TODO: Chain error reporting up to main
static long long get_file_size(const char * const file) {
    FILE* fp = fopen(file, "r");
    if (fp == NULL)  {
        perror ("Error opening file ahh");
        return(-1);
    }
    fseek(fp, 0, SEEK_END);
    long long len = ftell(fp);
    fclose(fp);
    return len;
}

int encode(int ref_size, const char * const infile, 
const char * const outfile) {
   
    // http://stackoverflow.com/questions/20863959/difference-between-opening-a-file-in-binary-vs-text
    FILE* in = fopen(infile, "rb");
    FILE* out = fopen(outfile, "wb");

    char c = 0;

    short itr = 0; 
    int bit = 0;
    uint64_t ref = 0;
    bool present = false;

    // Char array big enough to hold the reference
    char entry[get_num_bytes(ref_size)];

    dictionary dict; 
    dict.dict = init_lookup(ref_size);

    // This might be a bug in the distant future.
    // This is to avoid conflicts that point things to the 0th index, and
    // lets us reserve 0 to indicate no preceding character.
    dict.size = 1;

    while ((bit = get_next_bit(&c, itr, in)) != -1) {
        //printf("%d", bit);
        itr = (itr + 1) % 8;

        create_entry(entry, ref_size, ref, bit);

        // Get updated index if it exists in the table
        present = lookup(&dict, entry, &ref, ref_size);

        //printf(" was found: %s", present ? "true\n" : "false\n");

        if (!present) {
            store(&dict, entry, dict.size, ref_size);
            // This shouldn't be done here...
            fwrite(entry, 1, get_num_bytes(ref_size), out);
            ref = 0;
        }
        else if (present){
            // printf("ref is %" PRIu64 "\n" ,  ref);
            // Ref has been updated. Get next character and continue process
        }
    }
    if (ref != 0) {
        fwrite(entry, 1, get_num_bytes(ref_size), out);
    }
    fclose(in);
    fclose(out);
    free(dict.dict);
    return 0;
}

static int create_entry(char* entry, int ref_size, uint64_t ref, int bit) { 
    ref = ref << 1;
    ref = ref | bit;
    int num_bytes = get_num_bytes(ref_size);
    memcpy(entry, &ref, num_bytes); 

    // TODO: Add real value
    return 0;
}

// TODO: Apparently my bias for little endianness is bleeding everywhere...
// If I ever took this wider, I would have to address this
uint64_t bytes_to_int(char* c, int ref_size) {
    uint64_t total = 0; 
    int num_bytes = get_num_bytes(ref_size);
    memcpy(&total, c, num_bytes);
    return total;
}

uint64_t unpack_ref(char* c, int ref_size){
    uint64_t entry = bytes_to_int(c,ref_size);
    return entry >> 1;

}

int unpack_bit(char* c, int ref_size){
    uint64_t entry = bytes_to_int(c,ref_size);
    return entry & 1;
}

int get_num_bytes(int ref_size){
    return (ref_size + 1) / 8;
}

int add_to_byte(int* ctr, int bit, char* byte, FILE* out){
    if (*ctr == 8) {
        // Flush to buffer
        // reset byte
        // reset ctr
        //printf("Flushing: %d\n", *byte);
        fwrite(byte, 1, 1, out);
        *byte = 0;
        *ctr = 0;
        //printf("Flushed!\n");
    }
    //printf("Char was: %d\n", *byte);
    *byte = ((unsigned char)(*byte)) >> 1;
    //printf("Char is: %d\n", *byte);
    *byte = (*byte) | bit<<7;
    //printf("adding %d\n", bit);
    //printf("Before Counter is: %d\n", *ctr);
    *ctr = *ctr + 1;
    //printf("After Counter is: %d\n", *ctr);

    //TODO: Return real value
    return 0;
}

// I might be able to clean up the code in decode by just calling this
// method instead of repeating basically the same lines of code. Maybeee.
int walk_reference(dictionary* dict, int* ctr, uint64_t ref, int ref_size,
char* byte, FILE* out) {
    // Get entry from dictionary
    // Pull out bit
    // pull out ref
    // Add bit
    // If ref != 0
    //     walk_reference(...)

    int num_bytes = get_num_bytes(ref_size);
    uint64_t new_ref = 0;
    int bit = 0;
    char entry[get_num_bytes(ref_size)];

    memcpy(entry, dict->dict + (ref * num_bytes), num_bytes);

    new_ref = unpack_ref(entry, ref_size);
    bit = unpack_bit(entry, ref_size);

    //printf("walking, ref unpacked: %" PRIu64 "\n", new_ref);
    //printf("walking, bit unpacked: %d\n\n", bit);

    //printf("Counter is: %d\n", *ctr);
    if (new_ref != 0) {
        walk_reference(dict, ctr, new_ref, ref_size, byte, out);
    }
    add_to_byte(ctr, bit, byte, out);

    // TODO: Add real value
    return 0;
}

/**
 * Operation proceeds as follows:
 * 1) Initialize the dictionary
 * 2) Grab the next set of bytes from the compressed file
 * 3) Add the new entry to the database
 * 4) Unpack the reference and the bit
 * 5) Walk the new reference, incrementally building up a byte to flush
 *    (this is done automatically via another method)
 * 6) Repeat until the entire file has been decompressed
 */
int decode(int ref_size, const char * const infile, 
const char * const outfile) {

    FILE* in = fopen(infile, "rb");
    FILE* out = fopen(outfile, "wb");

    int num_bytes = get_num_bytes(ref_size);

    // I should really abstract this code into another method...
    dictionary dict; 
    dict.dict = init_lookup(ref_size);
    dict.size = 1;

    char entry[get_num_bytes(ref_size)];
    uint64_t ref;
    int bit;
    char out_byte = 0;
    int ctr = 0;

    // There should be an even multiple of num_bytes bytes in the file
    while (fread(entry, 1, num_bytes, in) != 0)
    {
        store(&dict, entry, dict.size, ref_size);
        ref = unpack_ref(entry, ref_size);
        bit = unpack_bit(entry, ref_size);
        //printf("ref unpacked: %" PRIu64 "\n", ref);
        //printf("bit unpacked: %d\n\n", bit);

        // Walk reference, will need to pass itr
        if (ref != 0) {
            walk_reference(&dict, &ctr, ref, ref_size, &out_byte, out);
        }
        add_to_byte(&ctr, bit, &out_byte, out);
    }
    // Flush the last byte. Might be able to re-organize so that we flush
    // if the counter goes over so we don't have to do this here.
    fwrite(&out_byte, 1, 1, out);

    fclose(in);
    fclose(out);
    free(dict.dict);


    // TODO: Add real value
    return 0;
}

/**
 * Operation proceeds as follows:
 * 1) Read the header from the infile to initialize parameters
 * 2) Perform inverse of parallel encode, creating a number of 
 *    pointers to the same infile and a number of outfiles
 *    to decompress the infile in parallel.
 * 3) As with parallel encode, merge the outfiles together.
 */
int parallel_decode(int num_threads, const char * const infile, 
    const char* const outfile) {

    FILE* in = fopen(infile, "rb");

    printf("Reading the header\n");
    header* h = read_header(in);

    long long fsize = get_file_size(infile);
    printf("Parallel Decode fsize: %llu\n", fsize);

    FILE** infiles  = malloc(h->num_threads * sizeof(FILE*));
    FILE** outfiles = malloc(h->num_threads * sizeof(FILE*));

    #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < h->num_threads; i++) {
      
        infiles[i] = fopen(infile, "rb");
        printf("split_locs: %ld\n", h->split_locs[i]);
        fseek(infiles[i], h->split_locs[i], SEEK_SET);

        long long end;
        if (i == h->num_threads - 1){
            end = fsize;	
        }
        else {
            end = h->split_locs[i + 1];
        }

        outfiles[i] = tmpfile();

        printf("Calling decode help:\n");
        printf("Start: %llu\n", h->split_locs[i]);
        printf("End:   %llu\n", end);
        decode_help(h->ref_size, h->split_locs[i], end, infiles[i], 
        outfiles[i]);

        fflush(outfiles[i]);
    }

    FILE* out = fopen(outfile, "wb+");

    merge_files(outfiles, h->num_threads, out);

    for (int i = 0; i < h->num_threads; i++) {
        fclose(infiles[i]);
        fclose(outfiles[i]);
    }

    free(infiles);
    free(outfiles);
 
    fclose(in);
    fclose(out);
    
    // TODO: Return actual error value
    return 0;
}

int decode_help(int ref_size, long long start, long long end, FILE* in, FILE* out) {

    int num_bytes = get_num_bytes(ref_size);

    // I should really abstract this code into another method...
    dictionary dict; 
    dict.dict = init_lookup(ref_size);
    dict.size = 1;

    char entry[get_num_bytes(ref_size)];
    uint64_t ref;
    int bit;
    char out_byte = 0;
    int ctr = 0;

    // Number of bits this will read
    long long bytes_to_get = (end - start);
    printf("Start: %llu\n", start);
    printf("end: %llu\n", end);
    printf("bytes_to_get: %llu\n", bytes_to_get);
    long long count = 0;

    //printf("Pointer %llu\n", ftell(in));
    // There should be an even multiple of num_bytes bytes in the file
    while (fread(entry, 1, num_bytes, in) != 0 && count < bytes_to_get)
    {
        //printf("Executing loop\n");
        count += num_bytes;
        store(&dict, entry, dict.size, ref_size);
        ref = unpack_ref(entry, ref_size);
        bit = unpack_bit(entry, ref_size);
        //printf("ref unpacked: %" PRIu64 "\n", ref);
        //printf("bit unpacked: %d\n\n", bit);

        // Walk reference, will need to pass itr
        if (ref != 0) {
            walk_reference(&dict, &ctr, ref, ref_size, &out_byte, out);
        }
        add_to_byte(&ctr, bit, &out_byte, out);
    }
    // Flush the last byte. Might be able to re-organize so that we flush
    // if the counter goes over so we don't have to do this here.
    fwrite(&out_byte, 1, 1, out);

    free(dict.dict);

    // TODO: Actual error checking
    return 0;
}


/**
 * (15 + 1) / 8 = 2 bytes
 * 2 bytes * 2^15 = number of entries
 */
char* init_lookup(int ref_size) {
    return malloc(get_num_bytes(ref_size) * (1 << ref_size));
}

bool lookup(dictionary* dict, char* entry, uint64_t* ref, int ref_size) {
    int num_bytes = get_num_bytes(ref_size);

    int val;
    uint64_t i;
    if (*ref == 0) {
        i = 1;
    }
    else{
        i = *ref;
    }
    for (; i < dict->size; i++){
        val = memcmp(entry, dict->dict + (i * num_bytes), num_bytes);
        if (val == 0) {
            *ref = i;
            return true;
        }
    }
    return false;
}

bool store(dictionary* dict, char* entry, uint64_t ref, int ref_size){
    if (dict->size < (uint64_t) (1 << ref_size)) {
        int num_bytes = get_num_bytes(ref_size);
        memcpy(dict->dict + (ref * num_bytes), entry, num_bytes);
        dict->size = dict->size + 1;
        return true;
    }
    return false;
}
