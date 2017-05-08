#/bin/bash

rm -rf test-out/*

for file in test-in/*;
do
    # run each with a variable number of threads
    for i in 1 2 4 8 16 32
    do
        # run command for compression, output time
        time ./lz78_omp compress $file "test-out/"$'compressed_'"$i""_""$(basename "$file")" $i 
        # output size of file after compression
        ls -lh "test-out/"$'compressed_'"$i""_""$(basename "$file")"
    done
done

for file in test-in/*;
do
    for i in 1 2 4 8 16 32
    do
        for j in 1 2 4 8 16 32
        do
            # run command for decompression, output time
            time ./lz78_omp decompress $"test-out/"$'compressed_'"$i""_""$(basename "$file")" $"test-out/"$'copy_'"$i""_""$j""_""$(basename "$file")" $i 
        done
    done
done

for file in test-in/*;
do
    for i in 1 2 4 8 16 32
    do
        for j in 1 2 4 8 16 32
        do
            # run make sure all files are the same
            diff $file $"test-out/"$'copy_'"$i""_""$j""_""$(basename "$file")"
        done
    done
done
