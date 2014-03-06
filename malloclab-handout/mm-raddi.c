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
 * Group: yupp
 * User 1: raggi
 * SSN: X
 * User 2: 
 * SSN: X
 * === End User Information ===
 ********************************************************/
team_t team = {
    /* Group name */
    "yupp",
    /* First member's full name */
    "Ragnar Omarsson", 
    /* First member's email address */
    "ragnaro08@ru.is",
    /* Second member's full name (leave blank if none) */
    "", 
    /* Second member's email address (leave blank if none) */
    "",
    /* Leave blank */
    "",
    /* Leave blank */
    ""
};

#define GET_HDR(bp) ((char *)bp - HDR_SIZE)

#define RET_BP(hdr_p) ((char *)hdr_p + HDR_SIZE) 

#define GET_SIZE_HDR(hdr) (hdr->size & ~1)
#define GET_SIZE_BP(bp) (GET_SIZE_HDR(GET_HDR(bp)))

#define GET_ALLOC_HDR(hdr) (hdr->size &0x1)

#define NEXT_ALLOC(hdr) ((((hdr_p)((char *)hdr + hdr->size + HDR_SIZE))->size) & 0x1)

#define NEXT_SIZE(hdr)  ((((hdr_p)((char *)hdr + hdr->size + HDR_SIZE))->size) & ~0x1)

#define NEXT_HDR(hdr)  ((hdr_p)((char *)hdr +ALIGN(hdr->size) + HDR_SIZE))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define HDR_SIZE (ALIGN(sizeof(hdr_p)))
#define CHUNK (1<<12)
#define WSIZE 4


typedef struct hdr* hdr_p; 
struct hdr{
    size_t size;
    hdr_p next;
    hdr_p prev;
};

void *find_fit(size_t size);
static hdr_p extend_heap(size_t words);
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    hdr_p startHdr;
    hdr_p heap;

    if((startHdr = mem_sbrk(HDR_SIZE)) == NULL){
        return -1;
    } 
    if((heap = extend_heap(CHUNK/WSIZE)) == NULL){
        return -1;
    } 

    startHdr->size = (0 | 1);
    startHdr->next = startHdr;
    startHdr->prev = startHdr;
    // heap->next = startHdr;
    // heap->prev = startHdr;
    
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(HDR_SIZE) + ALIGN(size);
    hdr_p pnt = find_fit(newsize);
    if(pnt == NULL){
        pnt = mem_sbrk(newsize);
        if((long)pnt == -1){
            return NULL;
        }
        else{
            pnt->size = newsize | 1;
        }
    }
    else {
        pnt->size |= 1;
        pnt->prev->next = pnt->next;
        pnt->next->prev = pnt->prev;
    }
   return ((char *)pnt + HDR_SIZE); 
}

void *find_fit(size_t size){
    hdr_p pnt = ((hdr_p)mem_heap_lo())->next;
    while(pnt != mem_heap_lo() && pnt->size < size){
        pnt = pnt->next;
    }

    if(pnt != mem_heap_lo()){
        return pnt;
    } 
    else{
        return NULL;
    } 
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{  
    hdr_p pnt = ptr - HDR_SIZE; 
    hdr_p heapStart = (hdr_p)mem_heap_lo();

    pnt->size &= ~1; 
    pnt->next = heapStart->next;
    pnt->prev = heapStart;
    heapStart->next = pnt;
    pnt->next->prev = pnt;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    hdr_p newp;
    hdr_p oldp;
    size_t copySize;
    size = ALIGN(size);

    oldp = (hdr_p)GET_HDR(ptr);
    hdr_p nxtp = NEXT_HDR(oldp);
    if(nxtp < (hdr_p)mem_heap_hi()){
        printf("%d allocation and size: %d\n",NEXT_ALLOC(oldp), nxtp->size);
        if(!GET_ALLOC_HDR(nxtp) &&  (nxtp->size + oldp->size + HDR_SIZE) >= size){
            oldp->size = nxtp->size + oldp->size + HDR_SIZE;
            nxtp->prev->next = nxtp->next;
            nxtp->next->prev = nxtp->prev;

            return ptr;
        }
    } 
    
    if((newp = mm_malloc(HDR_SIZE + size)) == NULL){
        printf("Error: mm_malloc failed in mm_realloc\n");
        exit(1);
    }
    copySize = oldp->size;
    if(copySize > size){
        copySize = size;
    }
    memcpy(((char *)newp + HDR_SIZE), ptr, copySize);
    newp->size = size | 1;
    mm_free(ptr);    

    return RET_BP(newp);
}

static hdr_p extend_heap(size_t words){
    
    hdr_p newMem;
    size_t size;

    /* allocate an even number of words */
    size = (words % 2) ? (words + 1)*WSIZE : words * WSIZE;
    if((newMem = mem_sbrk(HDR_SIZE + size)) == ((void *)-1)){
       return NULL;
    }
 
    newMem->size = (size &= ~1);
    newMem->next = NULL;
    newMem->prev = NULL;

    return newMem;
 
}
