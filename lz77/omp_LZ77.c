
#include <stdio.h>
#include "omp.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

// Runs OpenMP version of LZ77. 
// Trivial parallelsization by dividing the file to be compressed into num_threads number of partitions.
// The algorithm and choice of 11 bits for offset and 4 bits for length and 1 bit flag for LZSS taken from:http://michael.dipperstein.com/lzss/index.html
// Generates part_* files and a compressed .pw file for now. Decompression and compression use same number of threads due to offset issues. 
//TODO: Add option to take num_threads from command line.
// Figure out how to split compressed file while decompression.



int main(int argc, char *argv[])
{
    int flag, decompress = 0, infile = 1, outfile=2;
    unsigned char *buffer, *buffpos, *slidingwin, *winpos, *winsearch;
    int num_threads=4;//default
    unsigned long fileSize;
    unsigned short fileHeader;
    FILE *inputFile, *outputFile;
    omp_set_num_threads(num_threads);
    FILE** infiles  = malloc(num_threads * sizeof(FILE*));
    FILE** outfiles = malloc(num_threads * sizeof(FILE*));
    if (argc < 3 || argc > 5)
        return -1;
 
    if (strcmp(argv[1], "-d") == 0)
    {
     decompress = 1;
	infile=2;
	outfile=3;
    }
     

if (decompress){
    printf("Decompressing file. Num threads=%d...\n",num_threads);
    #pragma omp parallel for
    for (int i = 0; i < num_threads; i++) {	
	char num_buffer[33];
	char inname[strlen(argv[infile]) + 10];
	memset(inname, 0, strlen(argv[infile]) + 10);
	snprintf(num_buffer,33,"%d",i); 
	memcpy(inname, argv[infile], strlen(argv[infile])-3);
	strcat(inname, ".part_");
	strcat(inname, num_buffer);
	infiles[i] = fopen(inname, "rb");
        char num_buffer2[33];
	char outname[strlen(argv[outfile]) + 10];
	memset(outname, 0, strlen(argv[outfile]) + 10);
	snprintf(num_buffer2,33,"%d",i); 
	memcpy(outname, argv[outfile], strlen(argv[outfile])-4);
	strcat(outname,num_buffer2);
	strcat(outname,".txt");
	outfiles[i] = fopen(outname, "wb+");
	parallel_decompress(infiles[i], outfiles[i]);
	fflush(outfiles[i]);
	}
        outputFile = fopen(argv[outfile], "wb");
       //From Mitch. Hope it works without changes.
       char ch;
       for (int i = 0; i < num_threads; i++) {
        char num_buffer2[33];
	char outname[strlen(argv[outfile]) + 10];
	memset(outname, 0, strlen(argv[outfile]) + 10);
	snprintf(num_buffer2,33,"%d",i); 
	memcpy(outname, argv[outfile], strlen(argv[outfile])-4);
	strcat(outname,num_buffer2);
	strcat(outname,".txt");
	outfiles[i] = fopen(outname, "rb+");
        fseek(outfiles[i], 0, SEEK_SET);
        while((ch = fgetc(outfiles[i]) ) != EOF )
            fputc(ch,outputFile);
        remove(outname);
        }
   
}
else{ 
    printf("Compressing file. Num threads=%d...\n",num_threads);
    inputFile = fopen(argv[infile], "rb");
    fseek(inputFile, 0, SEEK_END);
    fileSize = ftell(inputFile);
    fclose(inputFile);
    unsigned long increment = fileSize / num_threads;
    //printf("%lu,%lu\n",increment,fileSize);
    #pragma omp parallel for
    for (int i = 0; i < num_threads; i++) {	
	infiles[i] = fopen(argv[infile], "rb");
	//fseek(infiles[i], i * increment, SEEK_SET);
        char num_buffer[33];
	char outname[strlen(argv[infile]) + 10];
	memset(outname, 0, strlen(argv[infile]) + 10);
	snprintf(num_buffer,33,"%d",i); 
	memcpy(outname, argv[infile], strlen(argv[infile])-4);
	//sprintf(outname,strlen(argv[infile]) - 4);
	strcat(outname, ".part_");
	strcat(outname, num_buffer);
	outfiles[i] = fopen(outname, "wb+");
	unsigned long end;
	if (i == num_threads - 1){
		end = fileSize;	
		}
	else {
	        end = (i + 1) * increment;
		}
	//printf("%d\t ",i);
	parallel_compress(i*increment, end, infiles[i], outfiles[i]);
	fflush(outfiles[i]);
	}  
        outputFile = fopen(argv[outfile], "wb");
        char ch;
        for (int i = 0; i < num_threads; i++) {
        char num_buffer[33];
	char outname[strlen(argv[infile]) + 10];
	memset(outname, 0, strlen(argv[infile]) + 10);
	snprintf(num_buffer,33,"%d",i); 
	memcpy(outname, argv[infile], strlen(argv[infile])-4);
	//sprintf(outname,strlen(argv[infile]) - 4);
	strcat(outname, ".part_");
	strcat(outname, num_buffer);
	outfiles[i] = fopen(outname, "rb+");
        fseek(outfiles[i], 0, SEEK_SET);
        while((ch = fgetc(outfiles[i]) ) != EOF )
            fputc(ch,outputFile);
        //remove(outname);
	}
    fclose(outputFile); 
}
return 0;
}



int parallel_compress(unsigned long start,unsigned long end, FILE* inputFile, FILE* outputFile)
{
    int flag=0, i, iLimit;
    unsigned char *buffer, *buffpos, *slidingwin, *winpos, *winsearch;
    unsigned long fileSize;
    unsigned short fileHeader;
    unsigned long count;
    count = start;
    fileSize=end-start;
    buffer = malloc(fileSize);
    buffpos = buffer;
    slidingwin = malloc(fileSize); 
    winpos = slidingwin;
    //printf("%lu,%lu,%lu\n",start,end,fileSize);
    //printf("%lu,%lu,%lu\n",buffpos,buffer,winpos);
    fseek(inputFile, start, SEEK_SET); //SEEK_SET 	Beginning of file
    fread(buffer, 1,fileSize, inputFile);
        while (buffpos - buffer < fileSize)
        {
            flag = 0;
            iLimit = winpos - slidingwin;
	    // encoding strings that offer less than one byte of savings, the minimum encoded length is three characters
            if (iLimit > 2)
            {
	     if (iLimit > 128)
		    {
                   iLimit = 128;
                   slidingwin+=15; //Some offset for the window
		    } 
		   i=3;
 		   for (winsearch=slidingwin;winsearch<=winpos;winsearch++)
                    {
 			while(i<iLimit)
            		{
			if ((winpos - winsearch) < i)
                                break;
                        if (memcmp(winsearch, buffpos, i) == 0)
			{
			   i++;
			   flag=1;
			}
		        else
		           break;
		        }
			if(flag)
			   break;
                   }
	       
            }
 
            if (!flag)
            {
		//printf("winpos\n");
                fileHeader = 0;
                fwrite(&fileHeader, sizeof(fileHeader), 1, outputFile);
                putc(*buffpos, outputFile);
                *(winpos++) = *(buffpos++);
            }
	    else{
                fileHeader = (unsigned long)0x8000 |((winpos-winsearch) << 7)|i-1;
                fwrite(&fileHeader, sizeof(fileHeader), 1, outputFile);
                buffpos += i-1;
		winpos += i-1;
		}
        }
    fclose(outputFile);    
    return 0;
}


int parallel_decompress(FILE* inputFile, FILE* outputFile)
{

    int flag, i, iLimit;
    unsigned char *buffer, *buffpos, *slidingwin, *winpos, *winsearch;
    unsigned long fileSize;
    unsigned short fileHeader;
    
    fseek(inputFile, 0, SEEK_END); 

    fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET); //SEEK_SET 	Beginning of file
    buffer = malloc(fileSize);
    buffpos = buffer;
    slidingwin = malloc(fileSize); 
    winpos = slidingwin;
    fread(buffer, fileSize, 1, inputFile);


        while (buffpos - buffer < fileSize)
        {   //offset and length are zero. 0x8000 in binary: 1 00000000 0000000
            fileHeader = *(unsigned short *)buffpos;
	    buffpos += 2;
            if (fileHeader & 0x8000) //Bitwise AND. Any thing >0x8000=> flag is 1.
            {     
	      //printf("%lu\n",(fileHeader & 0x7F80) >> 7);
	      fseek(outputFile,-(fileHeader & 0x7F80) >> 7,SEEK_END);
	      unsigned long len=(fileHeader & 0x7F);
              char *strng[len];
	      *strng = NULL;
	      fread(strng, len, 1, outputFile);
	      //printf("%lu\t%lu,\t%lu\n", buffpos ,winsearch,len);
	      //printf("%x \t %s \t %lu \n",fileHeader, strng, len);
	      fseek(outputFile,0,SEEK_END);
	      fwrite(strng, fileHeader & 0x7F, 1, outputFile);
	      //printf("Here\n");
            }
            else
            { 
	    // https://www.tutorialspoint.com/c_standard_library/c_function_putc.htm
	    // putc writes a character (an unsigned char) specified by the argument char.
                putc(*buffpos, outputFile);
                *(winpos++) = *(buffpos++);
            }

        }

    fclose(outputFile);    
    return 0;
}
