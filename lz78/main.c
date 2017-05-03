// Specify default parameters
// Serial/Parallel
// If parallel, partition size
// Dictionary size
// Open file
// 
// Should make sure that I have proper return values everywhere
// Store documentation in HEADER files.
// Need to make sure that every function in here is in the header file

// Oh holy fuck there is a difference between bit-level and byte-level
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "main.h"

void print_usage(char* prog_name) {
    printf("LZ78 Encryption Tool:\n");
    printf("Usage: %s infile outfile [dict_size]\n" , prog_name);
    printf("    infile:    file to compress\n");
    printf("    outfile:   file to write result to\n");
    printf("    dict_size: size, in bytes, of dictionary entry (1 or 2)\n");
}

int main(int argc, char** argv){

    if (argc != 3 && argc != 4) {
        print_usage(argv[0]);
        exit(1);
    }

    char* infile = argv[1];
    char* outfile = argv[2];
    int dict_size = 15;

    if (argc == 4) {
        printf("Got here\n");
        int size = strtol(argv[3], NULL, 10);
        printf("Got here\n");

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
    printf("Got here darn it\n");

    char temp[strlen(infile) + 2];
    memcpy(temp, infile, strlen(infile));
    strcat(temp, ".pw");

    // TODO: Error checking 
    encode(dict_size, infile, temp); 
    decode(dict_size, temp, outfile);

    // TODO: Remove input file

    return 0;
}

static int get_next_bit(char* c, uint64_t itr, FILE* in){
    if (itr == 0) {
        //printf("\n");
        //if (fgets(c, 2, in) == NULL) { 
        //    return -1;
        //}
        *c = fgetc(in);
        if (feof(in)){
            return -1;
        }
    }
    //printf("Character read: %c ",*c);
    int val = *c & 1;
    *c = *c >> 1;
    return val;
}

int encode(int ref_size, const char * const infile, 
const char * const outfile) {
   
    // http://stackoverflow.com/questions/20863959/difference-between-opening-a-file-in-binary-vs-text
    FILE* in = fopen(infile, "rb");
    FILE* out = fopen(outfile, "wb");

    char c;

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

    // Initialize dictionary CHECK
    // Loop:
    // Grab the next set of bytes CHECK
    // Add the entry to the database CHECK
    // unpack the reference and the bit CHECK
    // walk the reference, adding the bits to the byte. That function will
    // flush as necessary.
    // Add the bit to the byte
    // repeat.


    // TODO: Add real value
    return 0;
}


char* init_lookup(int ref_size) {
    // (15 + 1) / 8 = 2 bytes
    // 2 bytes * 2^15 = number of entries
    return malloc(get_num_bytes(ref_size) * (1 << ref_size));
}

// I can also optimize this by using ref to begin the search, instead of
// starting at 0 each time, which will aboslutely not bear fruit!
bool lookup(dictionary* dict, char* entry, uint64_t* ref, int ref_size) {
    int num_bytes = get_num_bytes(ref_size);

    // Look through the array and compare everything
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
            // printf("ref going out is %" PRIu64 "", *ref);
            return true;
        }
    }
    return false;
}

// Store function (takes key, value, stores it)
// Honestly, we should probably increment the store here...
// HONESTLY, I should probably refactor this. The dictionary does not need
// to be told what its size is. It should be able to figure that one out.
// ref == location in the dictionary to store the thing
bool store(dictionary* dict, char* entry, uint64_t ref, int ref_size){
    // I should make sure to check this logic
    if (dict->size < (1 << ref_size)) {
        int num_bytes = get_num_bytes(ref_size);
        memcpy(dict->dict + (ref * num_bytes), entry, num_bytes);
        dict->size = dict->size + 1;
        return true;
    }
    return false;
    //printf("Storing: %*.*s\n", num_bytes, num_bytes, entry);
}
