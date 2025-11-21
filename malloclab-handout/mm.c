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

#define HDRP(bp) ((char *)(bp) - sizeof(block_header))
#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1)) 

// BLOCK HEADER FOR ALLOCATED MEMORY TODO: PACK
typedef struct block_header { 
  size_t size;
  char allocated;
} block_header; 

// FREE LIST NODE
typedef struct node {
  struct node* prev;
  struct node* next; 
} node; 

#define HEADERSIZE sizeof(block_header) // 16 bytes 
#define NODESIZE sizeof(node)  // 16 bytes

node *first_node = NULL; 

// TODO: make block footer struct
// TODO make prolog and epilogue for coalescing (in each page), make sentinel terminator (for beginning and end of heap)

/*
* Helper functions
*/
void print_free_list_summary(void);
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void extend(size_t s);
void * add_node(void * ptr);
void * delete_node(void * ptr);
block_header* first_fit(size_t size);

/*
* Extend the heap size
*/
void extend(size_t s) {
  size_t new_size = PAGE_ALIGN(s);  // init more
    block_header * new_node = mem_map(new_size); 
    printf("%s %ld %s\n", "initialize_free_list: initialized", new_size, "bytes of memory\n");

    if (new_node == NULL) {
      printf("initialize_free_list: mem_map error\n");
      return;
    }
    
    // update free list now
    node * next = first_node;
    first_node = (node *) (new_node + 1);
    new_node->size = new_size - NODESIZE;
    new_node-> allocated = 0;
    first_node->next = next;
}


/* 
 * mm_init - initialize the malloc package. Not 100% sure what to do here
 * 
 * This method:
 * 1. creates the free list and allocates a free block using mem_map (call extend method)
 * 2. returns -1 on an error, 0 on success
 * 
 * pseudo code:
 * free list head = null
 * maybe keep track of # mapped pages
 * extend (1) -> centralizes calling memmap returns pointer to new block extend 1 because minimum is 4096 and we dont need morethan that bc we ewant good util
 *  split if possible
 *  allocate all space then turn rest into free block
 * 
 */
int mm_init(void)
{
  first_node = NULL;
  extend(1);
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
  size_t aligned_size = ALIGN(size);
  printf("malloc: size: %ld malloc: padded size with header: %ld\n", size, aligned_size);
  
  block_header* new_block = NULL;

  block_header* mem = first_fit(aligned_size);

  if ((!mem) || mem->size < aligned_size) {
    extend(aligned_size);
    new_block = (block_header *) first_node - 1;
  }
  else {
     new_block = mem;
  }

  node * new_block_node = (node *)new_block + 1;
  if (new_block_node && new_block_node-> prev) new_block_node-> prev = new_block_node->next;
  new_block->size = aligned_size;
  new_block->allocated = 1;
  
  // return pointer to new block (starting after header and node)
  return first_node + 1; 
  
}

block_header* first_fit(size_t size){
  node *curr = first_node; 
  while(curr != NULL && ((block_header *) HDRP(curr))-> size < size){
    printf("malloc: traversing this ho\n"); 
    curr = curr->next;
  }
  if(curr) return((block_header *) HDRP(curr));
  return NULL;
}

/*
* free block at ptr. No need to check if this is a block that we have access to! Just change the 
* block header to be allocated. TODO: add coalescing to this.
*/
void mm_free(void *ptr)
{
  // block_header * header = (block_header *)ptr; // go back to header make sure to ... fix this
  // header -> allocated = 0;
  // TODO: make it so it goes back in the free list
  // TODO coalesce
  // unmap page if it's all free if there are a decent amount of pages
}


/*
Helper to print all free list elements
*/
void print_free_list_summary(void){
  printf("free list summary:\n");
  node* ptr = first_node;
   while( ptr ) {
      printf( "block addr: %p\n", ptr);
      ptr = ptr->next;
   }
}

/*
NOTES:
- should call mem_map as little as possible (dont be calling it when we make a new list node)
- add to front of free list
*/
