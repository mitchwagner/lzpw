
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

// Runs serial version of LZSS. 
// The algorithm and choice of 11 bits for offset and 4 bits for length and 1 bit flag for LZSS taken from:http://michael.dipperstein.com/lzss/index.html
// Tried other combinations of offset and length, but decided to choose max. length of a match to 15. 


int main(int argc, char *argv[])
{
    int flag, match, decompress = 0, infile = 1, outfile=1, iLimit;
    int i;
    unsigned char *buffer, *buffpos, *slidingwin, *winpos, *winsearch;
    unsigned long fileSize;
    unsigned short fileHeader;
    unsigned char fileHeader_Small;
    FILE *inputFile, *outputFile;
    if (argc < 2 || argc > 4)
        return -1;
 
    if (strcmp(argv[1], "-d") == 0)
    {
        decompress = 1;
        infile = 2;
	outfile = 3;
    }
 
    inputFile = fopen(argv[infile], "rb");
    fseek(inputFile, 0, SEEK_END);
    fileSize = ftell(inputFile);
    //printf("%d\n",fileSize);
    //fseek to sets the file position of the input file "inputFile"
    //https://www.tutorialspoint.com/c_standard_library/c_function_fseek.htm
    fseek(inputFile, 0, SEEK_SET); //SEEK_SET 	Beginning of file
    buffer = malloc(fileSize);
    buffpos = buffer;
    slidingwin = malloc(fileSize); //slidingwin is the entire file, for now
    winpos = slidingwin;
    fread(buffer, fileSize, 1, inputFile);
    fclose(inputFile);
    if (decompress)
    {
        outputFile = fopen(argv[outfile], "wb");
	fclose(outputFile);
        outputFile = fopen(argv[outfile], "rb+");
	// until we find the end of file
        while (buffpos - buffer < fileSize)
        {   //offset and length are zero. 0x8000 in binary: 1 00000000 0000000
            fileHeader_Small = *(unsigned char *)buffpos;
	    //buffpos += 1;
	    //printf("%x\n",fileHeader);
            if (fileHeader_Small & 0x80) //Bitwise AND. Any thing >0x8000=> flag is 1.
            {
	      fileHeader = *(unsigned short *)buffpos;
	      //printf("%lu\n",(fileHeader & 0x7F00) >> 8);
	      fseek(outputFile,-(fileHeader & 0x7F00) >> 8,SEEK_END);
	      unsigned long len=(fileHeader & 0x7F);
              char *strng[len];
	      *strng = NULL;
	      fread(strng, len, 1, outputFile);
	      //printf("%lu\t%lu,\t%lu\n", buffpos ,winsearch,len);
	      //printf("%x \t %s \t %lu \n",fileHeader, strng, len);
	      fseek(outputFile,0,SEEK_END);
	      fwrite(strng, fileHeader & 0x7F, 1, outputFile);
	      buffpos=buffpos+2;
	      //printf("Here\n");
            }
            else
            {
	     //printf("here\n");
	    // https://www.tutorialspoint.com/c_standard_library/c_function_putc.htm
	    // putc writes a character (an unsigned char) specified by the argument char.
                putc(*buffpos, outputFile);
		//buffpos++;
                *(winpos++) = *(buffpos++);
            }

        }

 
    }
    else
    {
        sprintf(argv[infile] + strlen(argv[infile]) - 3, "pw");
        outputFile = fopen(argv[infile], "wb");

        while (buffpos - buffer < fileSize)
        {
            flag = 0;
            iLimit = winpos - slidingwin;
	    // encoding strings that offer less than one byte of savings, the minimum encoded length is three characters
            if (iLimit > 2)
            {
	     if (iLimit > 127)//due to endienness
		    {
                   iLimit = 127;
                   //slidingwin+=15; //Some offset for the window
		    } 
		   i=3;//printf("iLimit,%lu\twinpos,%lu\n",iLimit,winpos);
 		   for (winsearch=winpos-iLimit;winsearch<=winpos;winsearch++)
                    {//printf("winpos\n");
 			while(i<iLimit)
            		{
			if ((winpos - winsearch) < i)
                                break;
                        if (memcmp(winsearch, buffpos, i) == 0)
			{
			   i++;
			//printf("%d\t",i);
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
                //fileHeader_Small = 0;
                //fwrite(&fileHeader, sizeof(fileHeader), 1, outputFile);
                putc(*buffpos, outputFile);
                *(winpos++) = *(buffpos++);
            }
	    else{
                fileHeader = (unsigned long)0x8080 |((winpos-winsearch) << 8)|i-1;
		//printf("%x\n",fileHeader);
                fwrite(&fileHeader, sizeof(fileHeader), 1, outputFile);
                buffpos += i-1;
		winpos += i-1;
		}
        }
 
    }
    fclose(outputFile);    
    return 0;
}
