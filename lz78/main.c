// Specify default parameters
// Serial/Parallel
// If parallel, partition size
// Dictionary size
// Open file
// 
// Store documentation in HEADER files.

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
// [reference][1]
// Where the last bit of each entry is the new bit
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

int main(){
    encode(15,"hi","out.txt");
    return 0;
}

int get_next_bit(char* c, uint64_t itr, FILE* in){
    if (itr == 0) {
        printf("\n");
        if (fgets(c, 2, in) == NULL) { 
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
    // FILE* in = fopen(infile, "rb");
    FILE* in = fopen("test.txt", "rb");
    FILE* out = fopen(infile, "wb");

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
        printf("%d", bit);
        itr = (itr + 1) % 8;

        create_entry(entry, ref_size, ref, bit);

        // Get updated index if it exists in the table
        present = lookup(&dict, entry, &ref, ref_size);

        printf(" was found: %s", present ? "true\n" : "false\n");

        if (!present) {
            //printf("FALSE, ref is %" PRIu64 "\n" ,  ref);
            // Store (if possible) and output to file
            store(&dict, entry, dict.size, ref_size);
            // This shouldn't be done here...
            dict.size++;
            fwrite(entry, 1, get_num_bytes(ref_size), out);

            // Reset ref to 0
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
    // I will probably need some logic here, like "if ref is not 0, then dump
    // the reference. Otherwise, abort, abort! No more output.

    // main loop: get next bit CHECK
    // Initialize character stream. KEEP TRACK OF CUR_REF.
    // Get next bit. CHECK
    // Does it match anything in database? CALL LOOKUP, true/false;
    // Yes: get next character CHECK, KEEP LOOP ROLLING
    // No: put into database and output (SHOULD BE EASY)

    return 0;
}

// Some sort of function to get condensed version of reference?
// The problem is making the size of the value variable... 

int create_entry(char* entry, int ref_size, uint64_t ref, int bit) { 
    //printf("Before Shift %" PRIu64 "\n" ,  ref);
    //printf("Bit %d",  bit);
    ref = ref << 1;
    ref = ref | bit;

    //printf("Copying %" PRIu64 "\n" ,  ref);

    int num_bytes = get_num_bytes(ref_size);
    // This will likely take a bit of debugging to get right...
    memcpy(entry, &ref, num_bytes); 

    // Can delete when done debugging...
    //printf("Creating: %*.*s", num_bytes, num_bytes, entry);
}

/*

int unpack_ref(){}

int unpack_bit(){}

*/


int get_num_bytes(int ref_size){
    return (ref_size + 1) / 8;
}

int decode(int ref_size, const char * const infile, 
const char * const outfile) {

    int num_bytes = get_num_bytes(ref_size);

    // I should really abstract this code into another method
    dictionary dict; 
    dict.dict = init_lookup(ref_size);
    dict.size = 1;

    // Initialize dictionary
    // Grab the next number of bytes
    //

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
    uint64_t i = 1;
    for (i; i < dict->size; i++){
        int val = memcmp(entry, dict->dict + (i * num_bytes), num_bytes);
        if (val == 0) {
            *ref = i;
            printf("ref going out is %" PRIu64 "", *ref);
            return true;
        }
    }
    return false;
}

// Store function (takes key, value, stores it)
// Honestly, we should probably increment the store here...
int store(dictionary* dict, char* entry, uint64_t ref, int ref_size){
    int num_bytes = get_num_bytes(ref_size);
    memcpy(dict->dict + (ref * num_bytes), entry, num_bytes);
    //printf("Storing: %*.*s\n", num_bytes, num_bytes, entry);
}









