-------------------------------------------------------------------------------
  _      _____________          __
 | |    |___  /  __ \ \        / /
 | |       / /| |__) \ \  /\  / / 
 | |      / / |  ___/ \ \/  \/ /  
 | |____ / /__| |      \  /\  /   
 |______/_____|_|       \/  \/    
                                  
-------------------------------------------------------------------------------
                                  
This repository contains serial and parallel implementations of the classic
LZ77 and LZ78 compression algorithms.

Contributions:

LZ77 implemented by Aditya Pratapa
LZ78 implemented by Mitch Wagner

Notes:
- LZ78
    - Will only run (correctly) on little-endian machines
    - Requires OMP and pgcc 
    - Code is untested and fill likely fail on extremely small files, when 
      using more threads than bytes present in the file 

    - To build: 
        1) cd to lz78
        2) make lz78_omp

    - To run: "./lz78_omp" will display usage
              "./lz78_omp compress infile outfile [num_threads]" for general use
              

- LZ77
