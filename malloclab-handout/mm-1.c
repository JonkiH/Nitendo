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


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * Globla varibles 
 */
static char *FIRST = 0; // point at the start of the memmory.
void *TREE = 0;; // point at the first free memmory.

/*
 * Hellper function
 */

/*
 * return the best fit of free memmory, if not finding memmory 
 * return -1;
 */
void *findbestfit(size_t size, void *tree);

void freetree(void *ptr, void *tree);

void insertempty(void *ptr);
void *insertnode(void *ptr);
int size(void *ptr);
void *left(void *ptr);
void *down(void *ptr);
void *right(void *ptr);
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{   
    if(FIRST == 0) {
	FIRST = mem_heap_hi();
	if (FIRST == 0)
	     return -1;
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
  void *p;
    
  if ((p = findbestfit(newsize, TREE)) != 0);
  else 
    p = mem_sbrk(newsize);
    
  if (p == (void *)-1)
    return NULL;
  else {
    *(size_t *)p = (oversize << 1) + 1; // lshift by 1 and add the mark bit as used.
    void *lastptr = ((char *)p + (newsize - SIZE_T_SIZE));
    lastptr = p;
    //= &p; // point at begin
    return (void *)((char *)p + SIZE_T_SIZE);
  }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
   // set mark bit as unused
   memset (ptr, 0, sizeof(size_t));
   memset (((char *)ptr + SIZE_T_SIZE), 0 , sizeof(size_t)); 
   *(size_t *)((char *)ptr - SIZE_T_SIZE) -= 1;
   if (TREE == 0) {
     printf("tree node \n");
     TREE = ((char *)ptr - SIZE_T_SIZE);     
   }
   else {
     freetree(ptr - SIZE_T_SIZE, TREE);
     
   }
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



/*
 * Helperfunction implimentiations
 */
void *findbestfit(size_t size, void *tree) {
     /*
     * To DO
     */
    return 0;
}

void freetree(void *ptr, void *tree){
    printf("node %p %p ", tree, ptr);
    if (size(tree) == 0){
       printf("ble\n");
       memcpy(tree, &ptr, sizeof(size_t));
    }
    else {
      int compear = size(ptr) - size(tree);
      if (compear < 0){
         
	   printf("left ");
//	   memcpy(((char *)tree + SIZE_T_SIZE),&ptr, sizeof(size_t));
	 
         freetree(ptr, left(tree));
	 
      }
      else {
        
	   printf("right \n ");
//	   memcpy(((char *)tree + (SIZE_T_SIZE << 1)), ptr, SIZE_T_SIZE);
	
// 	   freetree(ptr, right(tree));
	 }
      }

}

/*
 * Insert two null pointers
 */
void insertempty(void *ptr){
  void *leftptr = ((char *)ptr + SIZE_T_SIZE);
  leftptr = (void *)0;
  void *rightptr = ((char *)ptr + (SIZE_T_SIZE << 1 ));
  rightptr = (void *)0;
}

int size(void *ptr) {
  return (*(size_t *)ptr >> 1);
}

void *left(void *ptr){
  return ((char *)ptr + sizeof(size_t));
}

void *down(void *ptr){
  return 0;
}

void *right(void *ptr){
  return ((char *)ptr + (sizeof(size_t) << 1));
}

