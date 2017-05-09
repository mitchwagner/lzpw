#/bin/bash



rm -rf test-out/*

COMPRESS_FILE="compress_stats.txt" 
DECOMPRESS_FILE="decompress_stats.txt"

echo "file, num_threads, secs, size" > compress_stats.txt
echo "file, num_threads_in, num_threads_out, secs" > decompress_stats.txt 

echo "Compressing using varying numbers of threads..."
for file in test-in/*;
do
     
    # run each with a variable number of threads
    for i in 1 2 4 8 16 32
    do
        echo -n "$(basename "$file")" >> $COMPRESS_FILE 

        echo -n "," >>  $COMPRESS_FILE

        echo -n "$i" >> $COMPRESS_FILE

        echo -n "," >> $COMPRESS_FILE

        # run command for compression, output time
        t=$( TIMEFORMAT="%R"; { time ./lz78_omp compress $file "test-out/"$'compressed_'"$i""_""$(basename "$file")" $i; } 2>&1 )

        echo -n "$t" >> $COMPRESS_FILE

        echo -n "," >> $COMPRESS_FILE

        # output size of file after compression
        (ls -l "test-out/"$'compressed_'"$i""_""$(basename "$file")") | awk '{print $5}' | head -c -1 >> $COMPRESS_FILE 
        echo "" >> $COMPRESS_FILE 
    done
done
echo "Done"

echo "Decompressing using varying numbers of threads..."
for file in test-in/*;
do
    for i in 1 2 4 8 16 32
    do
        for j in 1 2 4 8 16 32
        do
            echo -n "$(basename "$file")" >> $DECOMPRESS_FILE 
            echo -n "," >>  $DECOMPRESS_FILE
            echo -n "$i" >> $DECOMPRESS_FILE
            echo -n "," >> $DECOMPRESS_FILE
            echo -n "$j" >> $DECOMPRESS_FILE
            echo -n "," >> $DECOMPRESS_FILE

            echo "Decompressing file compressed with $i thread(s) with $j thread(s)"
            # run command for decompression, output time
            t=$( TIMEFORMAT="%R"; { time ./lz78_omp decompress $"test-out/"$'compressed_'"$i""_""$(basename "$file")" $"test-out/"$'copy_'"$i""_""$j""_""$(basename "$file")" $j; } 2>&1 )
            echo -n "$t" >> $DECOMPRESS_FILE
            echo -n "," >> $DECOMPRESS_FILE
            echo "" >> $DECOMPRESS_FILE 
        done
    done
done
echo "Done"

echo "Diff'ing to ensure equality..."
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
echo "Done"
