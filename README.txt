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

    - To build: 
        1) cd to lz78
        2) make lz78_omp

    - To run: run lz78_omp without any arguments for usage

- LZ77
