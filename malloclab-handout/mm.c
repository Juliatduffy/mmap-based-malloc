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

#define OVERHEAD (sizeof(block_header)+sizeof(block_footer)) // calculate overhead
#define HDRP(bp) ((char *)(bp) - sizeof(block_header)) // given bp, get the header or footer pointer
#define FTRP(bp) ((char *)(bp)+GET_SIZE(HDRP(bp))-OVERHEAD) // given bp, get the footer pointer
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp))) // get the next payload pointer
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-OVERHEAD)) // get the previous payload pointer

// ******These macros assume you are using a size_t for headers and footers ******
#define GET(p) (*(size_t *)(p))// get value at pointer 
#define PUT(p, val) (*(size_t *)(p) = (val)) // set value at pointer 
#define PACK(size, alloc) ((size) | (alloc)) // Combine a size and alloc bit
#define GET_ALLOC(p) (GET(p) & 0x1) // Given a header pointer get the allocation 
#define GET_SIZE(p) (GET(p) & ~0xF) // Given a header pointer get the size 
#define ALIGNMENT 16 // always use 16-byte alignment
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1)) // rounds up to the nearest multiple of ALIGNMENT
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1)) // rounds up to the nearest multiple of mem_pagesize()

typedef struct block_header { 
  size_t size;
  char allocated;
} block_header; 

typedef struct node {
  size_t value; // pointer to THIS memory
  size_t next; 
} node; 

// make prolog and epilog

#define HEADERSIZE sizeof(block_header) // 16 bytes 
#define NODESIZE sizeof(node)  // 16 bytes

node * first_node = {NULL, NULL}; // stores head of free linked list, ok to have explicit free list
int current_avail_size = 0;

/* 
 * mm_init - initialize the malloc package. NOt 100% sure what to do here
 */
int mm_init(void)
{
  // create free list
  // create empty free list 
  // extend the heap to create an initial free block
  first_node-> value = mem_map(4 * __WORDSIZE); // not sure where this wordsize thing came from
  first_node -> next = NULL;
  int current_avail_size = 0;

  return 0;
}

/* 
 * mm_malloc - Allocate a block by using bytes from new_block,
 *     grabbing a new page if necessary. need to split big block
 */
void *mm_malloc(size_t size)
{
  size += HEADERSIZE; 
  int newsize = ALIGN(size);

  if (current_avail_size < newsize) {
    current_avail_size = PAGE_ALIGN(newsize);
    current_avail = mem_map(current_avail_size);
    if (current_avail == NULL)
      return NULL;
  }
  
  // make header for new block
  block_header * curr_header = (block_header *) curr_block_ptr;
  curr_header->size = size;
  curr_header->allocated = 1;
 
  // update free list

  // return pointer to new block (starting after header)
  return (void *) (curr_header + 1);// question: +1 gives headersize more bytes right? or 1 word?
}

/*
 * mm_free
 */
void mm_free(void *ptr)
{
  // block_header * header = (block_header *)ptr;
  // header -> allocated = 0;
}

void extend(size_t s) {
// if we need more space call mem_map to get a new page
  free_list_tail = PAGE_ALIGN(s);
  void * new_block = mem_map(free_list_tail); // NEW PAGE
  if (new_block == NULL) {
    return NULL;
  }
  free_list_tail = new_block;
  }