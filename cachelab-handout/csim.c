/*
 Name: Jón Heiðar Erlendsson
 ID: jone11
 Emil: jone11@ru.is
*/
#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Helper function declitains */
void print_help(void);

int main(int argc, char *argv[])
{
    int hit = 100; // keeps number of hits.
    int miss = 0; // keeps number of misses.
    int evic = 0; // keeps number of evictions.
    int i; 	  // to use in for loobs.
    int s;	  // Number of set index bits (S = 2^s is the number of sets).
    int S; 	  // Number of sets.
    int E; 	  // Associativity (number of line per set).
    int b;	  // Number of block bits (B = 2^b is block size)
    int B;	  // Block size (bytes)
    int isv = 0;  // for verbose;
    char *t;	  // <tracefile>: Name of the valgrinf trace to replay
    FILE *file;	  // File
    char op; 	  // Operation denotes the type of memmory access.
    int addr;	  // Memmory address 
    int size; 	  // The size specifies the number of bytes accessed bt the operation.
    /* Check if the are to few varibles */
    if (argc < 9){
        print_help();
    }

    /* Insert to varibles and check if not right input */ 
    for (i = 1; i < argc; i++){
	if (strcmp(argv[i], "-v") == 0) isv = 1;
	else if ((strcmp(argv[i], "-s") == 0) && (++i < argc)){
	    s = atoi(argv[i]);
	    if (!s) print_help();
	}
	else if ((strcmp(argv[i], "-t") == 0) && (++i < argc)){
	    t = argv[i];
	}
	else if ((strcmp(argv[i], "-E") == 0) && (++i < argc)){
	    E = atoi(argv[i]);
	    if (!E) print_help();
	}
	else if ((strcmp(argv[i], "-b") == 0) && (++i) < argc){
	    b = atoi(argv[i]);
	    if (!b) print_help();
	}
	else if (strcmp(argv[i], "-h") == 0) print_help();
	else print_help();
    }
    
    /* Open file and error handlening */
    file = fopen(t, "r");
    if (file == NULL){
    	printf("Can't open file!!\n");
	print_help();
    }

    S = pow(2, s);
    B = pow(2, b);

    int temp; 
    while (fscanf(file, "%s %x,%x", &op, &addr, &size) != EOF) {
	if (temp == 0 || temp < addr){
	    temp = addr + B - 1;
	    miss++;
    	    printf("from file %s %d %x  MISS \n", &op, addr, size);
	}
	else {
	    hit++;
	    printf("from file %s %d %x  Hit \n", &op, addr, size);
	}

     }
     
    printf("arg hit %d\n", hit);
    printSummary(1000, miss, evic);
    fclose(file);
    return 0;
}

/* Prints out message to user about the program */
void print_help(void){
   printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
   printf("Options:\n");
   printf("  -h         Print this help message.\n");
   printf("  -v         Optional verbose flag.\n");
   printf("  -s <num>   Number of set index bits.\n");
   printf("  -E <num>   Number of lines per set.\n");
   printf("  -b <num>   Number of block offset bits.\n");
   printf("  -t <file>  Trace file.\n");
   printf(" Examples:\n");
   printf("   linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
   printf("   linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n" );
   exit(0);
}
