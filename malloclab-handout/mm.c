/*
 * mm-naive.c
 * author: Julia DUffy and CS4400 at the University of Utah
 * last edited: 11-20-2025
 * current implementation: explicit free list with no coalescing or splitting (not working)
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

#define ALIGNMENT 16 // always use 16-byte alignment
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1)) // rounds up to the nearest multiple of ALIGNMENT
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1)) // rounds up to the nearest multiple of mem_pagesize()

/// BLOCK HEADER FOR ALLOCATED MEMORY
typedef struct block_header {
    size_t size;                  
    int allocated;                    
    struct block_header *next; // next free block
    struct block_header *prev;    
} block_header;

// TODO make prolog and epilogue for coalescing
block_header * prolog;
block_header * epilogue;

#define HEADERSIZE sizeof(block_header)

block_header * first_node = NULL; // head of free list

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
    first_node->size = new_size - HEADERSIZE;
    first_node->next = NULL;
    first_node-> allocated = 0;
    first_node->prev = NULL;
  }

/* 
 * mm_init - initialize the malloc package. NOt 100% sure what to do here
 * 
 * This method:
 * 1. creates the free list and allocates a free block using mem_map (call extend method)
 * 2. returns -1 on an error, 0 on success
 */
int mm_init(void)
{
  initialize_free_list();
  if (!first_node)
    return -1;
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
  size += HEADERSIZE; 
  size_t new_size = ALIGN(size);
  printf("malloc: size: %ld\n", size);
  printf("malloc: padded size with header: %ld\n", new_size);

  block_header * curr = first_node;

  if (curr == NULL)  {
    printf("malloc: first_node points to no memory\n");
    return NULL;
  }

  // Now traverse our free list to find the next open spot (first fit)
  block_header * prev = curr->prev;
  while(curr && (curr->size < new_size)){
      printf("malloc: traversing this ho\n"); 
      prev = curr;
      curr = curr->next;
  }

  if (curr == NULL) {
    // get new page of memory
    size_t aligned_new_size = PAGE_ALIGN(new_size); 
    block_header * new_node = mem_map(aligned_new_size); 
    printf("malloc:'extend' allocated %ld new bytes at address: %p\n", aligned_new_size, new_node);

    if (new_node == NULL) {
      printf("ruh roh - mm_malloc");
      return NULL;
    }
    // update free list
    curr = new_node;
  }

  printf("malloc: adding block header for newly allocated memory at: %p\n", first_node);
  // make header for new block
  block_header* curr_header = (block_header *) (curr);
  curr_header->size = new_size - HEADERSIZE;
  curr_header->allocated = 1;
  curr_header-> prev = prev;
  if(prev) prev->next = curr_header-> next;

  printf("malloc: memory allocated at payload address: %p\n", curr_header + 1);
  return (void *) (curr_header + 1);
}

/*
 * free block at ptr. No need to check if this is a block that we have access to! Just change the 
 * block header to be allocated. TODO: add coalescing to this.
 */
void mm_free(void *ptr)
{
  block_header * header = (block_header *)ptr;
  header -> allocated = 0;
  if(header-> prev) header->prev->next = header->next;
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