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
 * Group: 
 * User 1: Jón Heiðar Erlendsson 
 * SSN: 1307794259
 * User 2: Ragnar Ómarsson 
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


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Globla varibles */
int *FIRST; // point at the start of the memmory.
int *FREE; // point at the first free memmory.


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{   
    if(FIRST == 0) {
	FIRST = mem_heap_hi();
	if (FIRST == 0)
	     return -1;
	FREE = FIRST;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    /*
     * Oversize is to make room for size of memmory and mark bit
     * and to make room for 2 pointers if asking for small chunk of memmory.
     */
    int oversize = size + 3;
    int newsize = ALIGN(oversize + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    //printf("The size is: %d, and the newsize is: %d\n", size, newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = oversize;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
   // printf("DELETING: %x \n", ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














