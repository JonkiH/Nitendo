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

typedef struct {
    int s;	// Number of set index bits (S = 2^s is the number of sets).
    long long S;// Number of sets.
    int E; 	// Associativity (number of line per set).
    int b;	// Number of block bits (B = 2^b is block size)
    long long B;// Block size (bytes)
    int isv;	// for verbose;
   
    int hits; 	// keeps number of hits.
    int miss;   // keeps number of misses.
    int evic; 	// keeps number of evictions.
} _parm;

typedef struct {
    int valid;
    long long tag;
    long long block;
} _line;

typedef struct {
    _line *line;
} _sets;

typedef struct {
    int S;
    _sets *sets;
} _cache;

/* Helper function declitains */
void data_load(_parm par, int addrs);
void data_store(_parm par, int addrs);
void data_modify(_parm par, int addrs);

_cache cache_init(_parm par);
void cache_clear(_cache cache);
void print_help(void);

int main(int argc, char *argv[])
{
    _parm par;
    _cache cache;
    int i; 	  // to use in for loobs.
    char *t;	  // <tracefile>: Name of the valgrinf trace to replay
    FILE *file;	  // File
    char op; 	  // Operation denotes the type of memmory access.
    int addr;	  // Memmory address 
    int size; 	  // The size specifies the number of bytes accessed bt the operation.
    /* Check if the are to few varibles */
    if (argc < 9){
        print_help();
    }

    par.isv = 0;
    par.hits = 0;
    par.miss = 0;
    par.evic = 0;
 
    /* Insert to varibles and check if not right input */ 
    for (i = 1; i < argc; i++){
	if (strcmp(argv[i], "-v") == 0) par.isv = 1;
	else if ((strcmp(argv[i], "-s") == 0) && (++i < argc)){
	    par.s = atoi(argv[i]);
	    if (!par.s) print_help();
	}
	else if ((strcmp(argv[i], "-t") == 0) && (++i < argc)){
	    t = argv[i];
	}
	else if ((strcmp(argv[i], "-E") == 0) && (++i < argc)){
	    par.E = atoi(argv[i]);
	    if (!par.E) print_help();
	}
	else if ((strcmp(argv[i], "-b") == 0) && (++i) < argc){
	    par.b = atoi(argv[i]);
	    if (!par.b) print_help();
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
    par.S = pow(2.0, par.s);
    par.B = 1 << par.b;
    cache = cache_init(par); 
    while (fscanf(file, "%s %x,%x", &op, &addr, &size) != EOF) {
	switch (op){
	    case 'L':
	        data_load(par, addr);
		break;
	    case 'S':
		data_store(par, addr);
		break;
	    case 'M':
		data_modify(par, addr);
		break;
	}
     }
     
    printSummary(par.hits, par.miss, par.evic);
    fclose(file);
    cache_clear(cache);
    return 0;
}

void data_load(_parm par, int addrs){
    printf("L\n");
}

void data_store(_parm par, int addrs){
    printf("S\n");
}

void data_modify(_parm par, int addrs){
    printf("M\n");
}


_cache cache_init(_parm par){
    _cache newCache;
    _sets set;
    _line line;
    newCache.sets = (_sets *)malloc(sizeof(_sets) * par.S);
    newCache.S = par.S;
    int i, n;
    for (i = 0; i < par.S; i++){
    	set.line = (_line *)malloc(sizeof(_line) * par.E);
	newCache.sets[i] = set;
	for (n = 0; n < par.E; n++){
	    line.valid = 0;
	    line.tag = 0;
	    line.block = 0;
	    set.line[n] = line;
	}
    }
    return newCache;
}

void cache_clear(_cache cache){
    int i;
    for (i = 0; i < cache.S; i++){
	if (cache.sets[i].line != NULL){
	    free(cache.sets[i].line);
	}
    }
    if (cache.sets != NULL) {
    	free(cache.sets);
    }
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
