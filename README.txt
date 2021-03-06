-------------------------------------------------------------------------------
  _      _____________          __
 | |    |___  /  __ \ \        / /
 | |       / /| |__) \ \  /\  / / 
 | |      / / |  ___/ \ \/  \/ /  
 | |____ / /__| |      \  /\  /   
 |______/_____|_|       \/  \/    
                                  
-------------------------------------------------------------------------------
                                  
This repository contains serial and parallel implementations of the classic
LZ77, LZSS, and LZ78 compression algorithms. LZ77 and LZSS are implemented
to provide byte-level compression; LZ78 is implemented to provide bit-level
compression.

Contributions:

LZ77 implemented by Aditya Pratapa
LZSS implemented by Aditya Pratapa
LZ78 implemented by Mitch Wagner

Notes:
- LZ78
    - Will only run (correctly) on little-endian machines
    - Requires OMP and pgcc 
    - Code is untested and fill likely fail on extremely small files, when 
      using more threads than bytes present in the file 

    - To build: 
        1) cd into lz78 
        2) "make lz78_omp"

    - To run: "./lz78_omp" will display usage
              "./lz78_omp compress/decompress infile outfile [num_threads]" for general use

    - To run testing framework: 
        1) Create folders "test-in" and "test-out" in "test" directory
        2) Place files to be used as benchmarks in "test-in" folder
        3) Compile and move the lz78_omp executable to the folder "test"
        4) Run test.sh in "test" (This may take a while depending on file size)
          
- LZ77
    - To build:
        1) cd into lz77
        2) "make"

    - To run:
        - Compression:   "./omp_lz77 test.txt test.pw"
        - Decompression: "./omp_lz77 -d test.pw test1.txt"

- LZSS
    - To build:
        1) cd into lzSS
        2) "make"

    - To run:
        - Compression:   "./omp_lzSS test.txt test.pw"
        - Decompression: "./omp_lzSS -d test.pw test1.txt"
