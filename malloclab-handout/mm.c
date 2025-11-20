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

// #define OVERHEAD (sizeof(block_header)+sizeof(block_footer)) // calculate overhead
// #define GET(p) (*(size_t *)(p))// get value at pointer 
// #define GET_SIZE(p) (GET(p) & ~0xF) // Given a header pointer get the size 
//  #define HDRP(bp) ((char *)(bp) - sizeof(block_header)) // given bp, get the header
// #define FTRP(bp) ((char *)(bp)+GET_SIZE(HDRP(bp))-OVERHEAD) // given bp, get the footer
// #define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp))) // get the next payload pointer
// #define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-OVERHEAD)) // get the previous payload pointer

// // ******These macros assume you are using a size_t for headers and footers ******
// #define PUT(p, val) (*(size_t *)(p) = (val)) // set value at pointer 
// #define PACK(size, alloc) ((size) | (alloc)) // Combine a size and alloc bit
// #define GET_ALLOC(p) (GET(p) & 0x1) // Given a header pointer get the allocation 
#define ALIGNMENT 16 // always use 16-byte alignment
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1)) // rounds up to the nearest multiple of ALIGNMENT
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1)) // rounds up to the nearest multiple of mem_pagesize()

/// BLOCK HEADER FOR ALLOCATED MEMORY
typedef struct block_header { 
  size_t size;
  char allocated;
} block_header; 

/// FREE LIST NODE
typedef struct node {
  size_t size; // size of this block
  void * next; 
  void * prev;  
  int filler;
} node; 

// TODO make prolog and epilogue for coalescing
block_header * prolog;
block_header * epilogue;

#define HEADERSIZE sizeof(block_header)
#define NODESIZE sizeof(node) 

node * first_node = NULL; // head of free list

/*
* helper for mm_init
*/
void initialize_free_list(void){
    size_t new_size = PAGE_ALIGN(4 * __WORDSIZE); 
    first_node = mem_map(new_size); 
    if (first_node == NULL) {
      printf("initialize_free_list: mem_map error\n");
      return;
    }
    // update free list now
    first_node->size = new_size - NODESIZE;
    first_node->next = NULL;
    first_node->prev = NULL;
  }

/* 
 * mm_init - initialize the malloc package. NOt 100% sure what to do here
 * 
 * This method:
 * 1. creates the free list and allocates a free block using mem_map (call extend method)
 * 2. returns -1 on an error
 */
int mm_init(void)
{
  initialize_free_list();
  return 0;
}

/* 
 *  mm_malloc - Allocate a block by using bytes from new_block,
 *  grabbing a new page if necessary. need to split big block
 *  
 *  This method
 *  1. aligns/pads the given size 
 *  2. traverses the free list until it finds an empty spot that is big enough or gets to the end of the list
 *  3. If we are at the end of the list, then  we call extend method (haven't implemented yet)
 *     to get a new page of heap memory of size 4096 bytes. Make sure to update free list accordingly.
 *  4. Then we create a new block header for the memory we are allocating
 *  5. Then return the pointer to the new memory we just allocated.
 */
void *mm_malloc(size_t size)
{
  printf("malloc(%ld)\n", size);
  printf("malloc: first-node points to: %p\n", first_node);
  printf("malloc: first-node->next points to: %p\n", first_node->next);
  size += HEADERSIZE; 
  size_t new_size = ALIGN(size);
  printf("malloc: padded size with header: %ld!\n", new_size);

  node * curr = first_node;

  if (curr == NULL)  {
    printf("malloc: first_node points to no memory\n");
    return NULL;
  }

  // Now traverse our free list to find the next open spot (first fit)
  node * prev = curr-> prev;
  while(curr && (curr->size < new_size)){
      printf("malloc: traversing this ho\n"); // print begining and end of this method what all values are to see if they change unexpectedly
      prev = curr;
      curr = curr->next;
  }

  if (curr == NULL) {
    printf("malloc: extend 1\n");
    // make new block of memory, make new pointer
    size_t aligned_new_size = PAGE_ALIGN(new_size); 
    node * new_node = mem_map(aligned_new_size); 
    printf("malloc: extend 2\n");

    // something went wrong with mem_map
    if (new_node == NULL) {
      printf("ruh roh - mm_malloc");
      return NULL;
    }
    printf("malloc: extend 3\n");
    // update free list now
    curr = new_node;
    prev-> next = curr;
    curr-> prev = prev;
    curr-> next = NULL;
    curr-> size = new_size;

    printf("malloc: extend 4\n");
  }

  printf("malloc: got new block, time to add block header\n");
  printf("malloc: first-node now points to: %p\n", first_node);
  // make header for new block
  block_header* curr_header = (block_header *) (curr); // store block header at the new allocated memory after node
  curr_header->size = new_size  - HEADERSIZE;
  curr_header->allocated = 1;
  printf("malloc: first-node + block header now points to: %p\n", curr_header + 1);

  // return pointer to new block (starting after header bc user would overwrite header otherwise)
  return (void *) (curr_header + 1); // FIXME: is + HEADERSIZE ok here? there might be a macro for this I could use
return 0;
}

/*
 * free block at ptr. No need to check if this is a block that we have access to! Just change the 
 * block header to be allocated. TODO: add coalescing to this.
 */
void mm_free(void *ptr)
{
  block_header * header = (block_header *)ptr;
  header -> allocated = 0;
  // TODO: implement this
}

/*
* Extend the heap size
*/
void extend(size_t s) {
  // TODO: implement this
}


/*
NOTES:
- should call mem_map as little as possible (dont be calling it when we make a new list node)
- should add prolog and epilog block for coalescing
- without splitting, right now every block of memory is it's own heap page/area (bad)
- first fit > best fit for this assignment according to TA
*/