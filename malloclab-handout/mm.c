/*
 * mm-naive.c - The least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by allocating a
 * new page as needed.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

/* always use 16-byte alignment */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

void *curr_block_ptr = NULL;
int curr_block_size = 0;
void * head;

#define HEADERSIZE 32 // yuck lol

typedef struct block_header { 
  size_t size;
  struct block_header* next;
  char allocated;
  struct block_header* prev; 
} block_header;  // 32 bytes total

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  // ptr to most recently allocated block
  curr_block_ptr = NULL;
  // size of most recently allocated block
  curr_block_size = 0;
  return 0;
}

/* 
 * mm_malloc - Allocate a block by using bytes from new_block,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{
  int newsize = ALIGN(size);
   void * prev = NULL;
  if(curr_block_ptr != NULL) {
    prev = curr_block_ptr - HEADERSIZE;
  }

  // if we need more space call mem_map to get a new page
  if (curr_block_size < newsize) {
    curr_block_size = PAGE_ALIGN(newsize);
    void * new_block = mem_map(curr_block_size); // NEW PAGE
    if (new_block == NULL) {
      return NULL;
    }
    if (curr_block_ptr == NULL) {
      head = new_block;
    }
    curr_block_ptr = new_block;
    }

  curr_block_ptr  += newsize + HEADERSIZE;
  curr_block_size -= newsize; // don't understand this
  
  // make header for new block
  block_header * curr_header = (block_header *) curr_block_ptr;
  curr_header->size = size;
  curr_header->allocated = 1;
  curr_header->prev = prev;
 
  // set "next" pointer for previous block
  if (prev != NULL) {
    block_header* prev_header = (block_header *) prev;
    prev_header-> next = curr_header;
  }

  // return pointer to new block (starting after header)
  return curr_block_ptr;
}

/*
 * mm_free
 */
void mm_free(void *ptr)
{
  // block_header * header = (block_header *)ptr;
  // header -> allocated = 0;
}
