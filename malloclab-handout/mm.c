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
#define FTRP(bp)       ((char *)(bp) + ALIGN(GET_SIZE(HDRP(bp))))

/* Given header ptr p, compute address of its block ptr bp*/
#define BLPTR(p)       ((char *)p + HDR_SIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + ALIGN(GET_SIZE(HDRP(bp))) + OVERHEAD)
#define PREV_BLKP(bp)  ((char *)bp - GET_FSIZE(HDRP(bp) - FDR_SIZE) - OVERHEAD)

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
      	p->size = PACK(0, 1);
      	p->next=p;
      	p->prev=p;
      	PUT(((char *)p+GET_SIZE(p)-FDR_SIZE), GET_SIZE(p));      
    }
//    if ((heap_listp = mem_sbrk(4*WSIZE)) == NULL)
//        return -1;
//    PUT(heap_listp, 0);                        /* alignment padding */
//    PUT(heap_listp+WSIZE, PACK(OVERHEAD, 1));  /* prologue header */
//    PUT(heap_listp+DSIZE, PACK(OVERHEAD, 1));  /* prologue footer */
//    PUT(heap_listp+WSIZE+DSIZE, PACK(0, 1));   /* epilogue header */
//    heap_listp += DSIZE;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
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
//    size_t extendsize; /* amount to extend heap if no fit */
    hdr_p p, frep;
    void *bp;
    /* Ignore spurious requests */
    if (size <= 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = ALIGN(DSIZE + OVERHEAD);
    else
        asize = ALIGN(DSIZE * ((size + (OVERHEAD) + (DSIZE-1)) / DSIZE));
    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
//        bp = BLPTR(p);
//        printf("found ");
		place(bp, asize);
        return bp;
    }
    else {
//		printf("not found ");
        /* No fit found. Get more memory and place the block */
//        extendsize = MAX(asize,CHUNKSIZE);
		size_t s = MAX(asize, CHUNKSIZE);
		p = (hdr_p)mem_sbrk(s);
        if ((long)p == -1)
            return NULL;
        frep = (hdr_p)mem_heap_lo();
		p->next = frep->next;
		frep->next = p;
		p->prev = frep;
		p->size = s - OVERHEAD - FDR_SIZE;
		p->next->prev = p;
		bp = BLPTR(p);
        place(bp, asize);
        return bp;
    }
}
/* $end mmmalloc */

/* 
 *  * mm_free - Free a block 
 *   */
/* $begin mmfree */
void mm_free(void *bp)
{ 
    size_t size = GET_SIZE(HDRP(bp));
    ((hdr_p)HDRP(bp))->size = PACK(size, 0);
    PUT(FTRP(bp), PACK(size, 0));
	coalesce(bp);
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
 *  * extend_heap - Extend heap with free block and return its block pointer
 *   */
/* $begin mmextendheap */
/*static void *extend_heap(size_t words)
{
    hdr_p p;
    size_t size;

    / Allocate an even number of words to maintain alignment /
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((p = mem_sbrk(size)) == NULL)
        return NULL;
//    hdr_p p = (hdr_p)bp;
    p->size=size;
    p->next=p;
    / Initialize free block header/footer and the epilogue header */
//    bp->size=size;
//    bp->next=bp;
//    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
//    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
//    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */
/*
    / Coalesce if the previous block was free /
    return coalesce(p);
}*/
/* $end mmextendheap */

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
//    printf("csize=%d asize=%d ble=%d\n", csize, asize, ALIGN(DSIZE+OVERHEAD)); 
    if ((csize - asize) >= ALIGN(DSIZE)) {
//		printf("kill \n");
		hdr_p p = (hdr_p)HDRP(bp);
		p->size=PACK(asize-OVERHEAD, 1);
//		PUT(HDRP(bp), PACK(asize, 1));
//    	PUT(FTRP(bp), PACK(asize, 1));
    	bp = NEXT_BLKP(bp);
		hdr_p np = HDRP(bp);
		np->next=p->next;
		np->prev=p->prev;
		p->next->prev=np;
		p->prev->next=np;

//  PUT(HDRP(bp), PACK(csize-asize, 0));
//  PUT(FTRP(bp), PACK(csize-asize, 0));
    }
    else {
//		printf("ans \n");
 		hdr_p p = (hdr_p)HDRP(bp);
		p->next->prev = p->prev;
		p->prev->next = p->next;
		p->size = PACK(asize-OVERHEAD,1);
		

//        PUT(HDRP(bp), PACK(csize, 1));
//        PUT(FTRP(bp), PACK(csize, 1));
    }

}
/* $end mmplace */

/* 
 *  * find_fit - Find a fit for a block with asize bytes 
 *   */
static void *find_fit(size_t asize)
{
    /* first fit search */
    hdr_p p;
    for (p=((hdr_p)mem_heap_lo())->next;
         p != mem_heap_lo() && p->size < asize;
	 	 p = p->next);
    if (p != mem_heap_lo())
       return (char *)BLPTR(p);

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
	printf("1 \n");
	p = (hdr_p)HDRP(bp);
		printf("2, %x, %X \n", p->size, GET_ALLOC(p));
	np = (hdr_p)HDRP(NEXT_BLKP(bp));
		printf("3, %x, %x \n", np->size, GET_ALLOC(np));
	pp = HDRP(PREV_BLKP(bp));
    
	 		printf("4, %x, %x \n", pp->size);
	size_t prev_alloc = GET_ALLOC(pp);
    	printf("5 \n");
	size_t next_alloc = GET_ALLOC(np);
    	printf("6 \n");
	size_t size = GET_SIZE(HDRP(bp));

    if ((prev_alloc && next_alloc) || (!np->size && !pp->size)) {            /* Case 1 */
        printf("none \n");
		fp = (hdr_p)mem_heap_lo();
		p->next = fp->next;
		p->prev = fp->prev;
//		p->next->prev = p;
//		p->prev->next = p;
		return bp;
    }

    else if ((prev_alloc && !next_alloc) || (!pp->size && !next_alloc)) {      /* Case 2 */
		printf("next \n");
		size += GET_SIZE(np);
		p->next = np->next;
		p->prev = np->prev;
		p->prev->next = p;
		p->next->prev = p;
        PUT(HDRP(bp)->size, PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
    }

    else if ((!prev_alloc && next_alloc) || (!np->size && !prev_alloc)) {      /* Case 3 */
        printf("prev \n");
		size += GET_SIZE(pp);
// 		pp->next = p->next;
//		pp->prev = p->prev;
//		p->prev->next = pp;
//		p->next->prev = pp;
		pp->size = PACK(size, 0);
//	    PUT(FTRP(BLPTR(pp)), PACK(size, 0));
        bp = BLPTR(pp);
    }

    else {                                     /* Case 4 */
		printf("both \n");
//		pp->next = np->next;
//		pp->prev = np->prev;
//		pp->prev->next = pp;
//		pp->next->prev = pp;
 
        size += GET_SIZE(np) + GET_SIZE(pp);
		pp->size = size;
        PUT(FTRP(BLPTR(pp)), PACK(size, 0));
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


