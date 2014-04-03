/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in below _AND_ in the
 * struct that follows.
 *
 * === User information ===
 * Group: Nitendo 
 * User 1: jone11	 
 * SSN: 1307794259
 * User 2: ragnaro08 
 * SSN: 2903842579
 * === End User Information ===
 ********************************************************/
team_t team = {
    /* Group name */
    "Nitendo",
    /* First member's full name */
    "Jón Heiðar Erlendsson",
    /* First member's email address */
    "jone11@ru.is",
    /* Second member's full name (leave blank if none) */
    "Ragnar Ómarsson",
    /* Second member's email address (leave blank if none) */
    "ragnaro08@ru.is",
    /* Leave blank */
    "",
    /* Leave blank */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* Basic constants and macros */
#define WSIZE       4       /* word size (bytes) */  
#define DSIZE       8       /* doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)  /* initial heap size (bytes) */
#define OVERHEAD    HDR_SIZE+FDR_SIZE      /* overhead of header and footer (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(size_t *)p)
#define PUT(p, val)  (*(size_t *)p = (val))  

/*
(which is about 54/100). Read the size and allocated fields from address p */
#define GET_SIZE(p)  (p->size & ~0x7)
#define GET_FSIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (p->size & 0x1)
#define GET_FALOC(p) ((char *)p & 0x1)
#define SET_SIZE(p, val)  (p->size = val)  

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((hdr_p)((char *)(bp) - HDR_SIZE))  
#define FTRP(bp)       ((char *)(bp) + ALIGN(GET_SIZE(HDRP(bp))) - OVERHEAD)

/* Given header ptr p, compute address of its block ptr bp*/
#define BLPTR(p)       ((char *)p + HDR_SIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + ALIGN(GET_SIZE(HDRP(bp))))
#define PREV_BLKP(bp)  ((char *)bp - GET_FSIZE(HDRP(bp) - FDR_SIZE) )

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define HDR_SIZE (ALIGN(sizeof(struct Header)))
#define FDR_SIZE 8
/* $end mallocmacros */

typedef struct Header *hdr_p;
struct Header {
  	size_t size;
  
  	hdr_p next;
  	hdr_p prev;
};
/* Global variables */
//static char *heap_listp;  /* pointer to first block */

/* function prototypes for internal helper routines */
//static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkblock(void *bp);

/* 
 * mm_init - initialize the malloc package.
 */
/* $begin mminit */
int mm_init(void)
{   
    /* create the initial empty heap */
	hdr_p p;
    if ((p = mem_sbrk(ALIGN(OVERHEAD))) == NULL){
      return -1;
    }
    else {
      	p->size = PACK(OVERHEAD, 1);
      	p->next=NULL;
      	p->prev=p;
      	PUT(BLPTR(p), GET_SIZE(p));      
    }
    return 0;
}
/* $end mminit */

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size)
{
    size_t asize;      /* adjusted block size */
    hdr_p p;
    void *bp;
    if (size <= 0)
        return NULL;
    if (size <= DSIZE)
        asize = ALIGN(DSIZE + OVERHEAD);
    else
        asize = ALIGN(size + OVERHEAD);
    if ((p = find_fit(asize)) != NULL){
	    p->size |= 1;
//		PUT(FTRP(BLPTR(p)), p->size);
		p->prev->next = p->next;
		if (p->next != NULL)
			p->next->prev = p->prev;
	}
    else {
		p = (hdr_p)mem_sbrk(asize);
	    if ((long)p == -1)
            return NULL;
		p->size = asize | 1;
		PUT(FTRP((BLPTR(p))), p->size);
     	bp = BLPTR(p);
    }
    
    return bp;
}
/* $end mmmalloc */

/* 
 *  * mm_free - Free a block 
 *   */
/* $begin mmfree */
void mm_free(void *bp)
{
	hdr_p pnt = bp - HDR_SIZE;
    hdr_p heapStart = (hdr_p)mem_heap_lo();

	pnt->size &= ~1;
	pnt->next = heapStart->next;
	pnt->prev = heapStart;
	heapStart->next = pnt;
	pnt->next->prev = pnt;
/*
    hdr_p fp,p;
	p = (hdr_p)HDRP(bp);
   	fp = (hdr_p)mem_heap_lo();
	size_t size = GET_SIZE(p);
    ((hdr_p)HDRP(bp))->size = PACK(size,0);
    PUT(FTRP(bp), PACK(size, 0));
	fp = (hdr_p)mem_heap_lo();
  	p->prev = fp;
	p->next = fp->next;
	if (fp->next != NULL)
		fp->next->prev = p;
    fp->next = p;		*/
//    p->next = fp->next;
//	p->prev = fp;
//    fp->next = p;
//	printf(" DONE \n");
//	coalesce((char *)bp);
}

/* $end mmfree */

/*
 elloc - naive implementation of mm_realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
/*
    void *newp;
    size_t copySize;

    if ((newp = mm_malloc(size)) == NULL) {
        printf("ERROR: mm_malloc failed in mm_realloc\n");
        exit(1);
    }
    copySize = GET_SIZE(HDRP(ptr));
    if (size < copySize)
      copySize = size;
    memcpy(newp, ptr, copySize);
    mm_free(ptr);
    return newp;
*/
return NULL;
}


/* 
 *  * mm_checkheap - Check the heap for consistency 
 *   */
void mm_checkheap(int verbose)
{
/*
    char *bp = heap_listp;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose)
            printblock(bp);
        checkblock(bp);
    }

    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
*/
}

/* The remaining routines are internal helper routines */


/* 
 *  * place - Place block of asize bytes at start of free block bp 
 *   *         and split if remainder would be at least minimum block size
 *    */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
    size_t csize = GET_SIZE(HDRP(bp));
	if ((csize - asize) >= ALIGN(DSIZE)) {
		hdr_p p, fp;
		p = HDRP(bp);
		p->prev->next = p->next;
		if (p->next == NULL)
			p->next->prev = p->prev;
        fp = (hdr_p)mem_heap_lo();
		p->size=PACK(asize, 1);
        
    	PUT(FTRP(BLPTR(p)), p->size);
    	bp = NEXT_BLKP(bp);
		/*
		np->next = fp->next;
        fp->next->prev = fp; 
		np->prev = fp->prev;
		np->size = PACK((csize - asize), 0);
		PUT(FTRP(BLPTR(np)), np->size);
*/
    }
    else {
 		hdr_p p = (hdr_p)HDRP(bp);
        if (p->next != NULL){
			p->next->prev == p->prev;
		}
		p->prev->next = p->next;
		p->size = PACK(asize-OVERHEAD,1);
	    PUT(FTRP(BLPTR(p)), p->size);	
    }
}
/* $end mmplace */

/* 
 *  * find_fit - Find a fit for a block with asize bytes 
 *   */
static void *find_fit(size_t asize)
{
    /* first fit search */
    hdr_p p = (hdr_p)mem_heap_lo();
	if (p->next == NULL){
		return NULL;
	}
    for (p=p->next;
       	 p != mem_heap_lo() && GET_SIZE(p) < asize && p->next != NULL;
	 	 p = p->next);
    if (p != mem_heap_lo()){
 
    	return BLPTR(p);
	}
    	
/*
    void *bp;
    
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
*/
    return NULL; /* no fit */
}


/*
 *  * coalesce - boundary tag coalescing. Return ptr to coalesced block
 *   
 */
static void *coalesce(void *bp)
{
    hdr_p p, np, pp, fp;
	p = (hdr_p)HDRP(bp);
    
	np = (hdr_p)HDRP(NEXT_BLKP(bp));
	pp = (hdr_p)HDRP(PREV_BLKP(bp));
    
	size_t prev_alloc = GET_ALLOC(pp);
	size_t next_alloc = GET_ALLOC(np);
	size_t size = GET_SIZE(HDRP(bp));
	
    if ((prev_alloc && next_alloc) 
         || (np->size == 0 && pp->size ==0)) {            /* Case 1 */
	fp = (hdr_p)mem_heap_lo();
	p->next = fp->next;
	p->prev = fp->prev;
	fp->next = p;
	fp->prev->next = p;
//		return bp;
    }

    else if ((prev_alloc && !next_alloc) 
             || (pp->size ==0 && !next_alloc)) {      /* Case 2 */
	size += GET_SIZE(np) + OVERHEAD;
	p->next = np->next;
	p->prev = np->prev;
//	np->prev->next = p;
//	np->next->prev = p;
    p->size = PACK(size, 0);
    PUT(FTRP(bp), PACK(size,0));
    }

    else if ((!prev_alloc && next_alloc) 
              || (np->size == 0 && !prev_alloc)) {      /* Case 3 */
	    size += GET_SIZE(pp) + OVERHEAD;
 	    pp->next = p->next;
	    pp->prev = p->prev;
	    p->prev->next = pp;
	    p->next->prev = pp;
	    pp->size = PACK(size, 0);
        PUT(FTRP(BLPTR(pp)), pp->size);
        bp = BLPTR(pp);
    }

    else {                                     /* Case 4 */
		pp->next = np->next;
		pp->prev = np->prev;
//		pp->prev->next = pp;
//		pp->next->prev = pp;
 
        size += GET_SIZE(np) + GET_SIZE(pp) + (OVERHEAD << 1);
		pp->size = PACK(size,0);
        PUT(FTRP(BLPTR(pp)), pp->size);
        bp = BLPTR(pp);
    }

    return bp;
}

static void printblock(void *bp)
{
//    size_t hsize, halloc, fsize, falloc;
        
/*    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp,
           hsize, (halloc ? 'a' : 'f'),
           fsize, (falloc ? 'a' : 'f'));
*/
}

static void checkblock(void *bp)
{
/*    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
*/
}


