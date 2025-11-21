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

// BLOCK HEADER FOR ALLOCATED MEMORY TODO: PACK to 8 bytes
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

// TODO: make 8 byte block footer struct
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
  printf("extend invoked -----------------------------------\n");
  size_t new_size = PAGE_ALIGN(s);  
    block_header * new_node = mem_map(new_size); 
    printf("%s %ld %s\n", "extend: initialized", new_size, "bytes of memory");

    if (new_node == NULL) {
      printf("extend: mem_map error\n");
      return;
    }
    
    // update free list now
    node * next = first_node;
    printf("%s %p\n", "extend: old first node:", next);
    first_node = (node *) (new_node + 1); // new node / new memory + 1 to account for block_header
    printf("%s %p\n", "extend: new first node:", first_node);
    block_header * first_block_header = (block_header *) HDRP(first_node);
    first_block_header->size = new_size - NODESIZE;
    printf("%s %ld\n", "extend: new first hdrp size (should be 4080):", first_block_header->size);
    first_block_header->allocated = 0;
    printf("%s %d\n", "extend: new first hdrp allocated (should be 0):", first_block_header->allocated);
    first_node->next = next;
    printf("%s %p\n", "extend: new first node-> next (old first_node):", first_node->next);
    first_node->prev = NULL;
    printf("%s %p\n", "extend: new first node-> prev (should be nil):", first_node->prev);
    printf("extend: returned successfully\n\n");
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
  printf("\nmm_init invoked -----------------------------------\n");
  first_node = NULL;
  extend(1);
  if (!first_node) {
    printf("mm_init: returned successfully\n\n");
    return -1;
  }
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
  printf("mm_malloc invoked -----------------------------------\n");
  // get aligned (by 4096) size, accounting for overhead
  size += HEADERSIZE; 
  size_t aligned_size = ALIGN(size);
  printf("malloc: size: %ld padded size with header: %ld\n", size, aligned_size);
  
  // find a free block or determine that there are no free blocks
  block_header* free_block = NULL;
  block_header* mem = first_fit(aligned_size);

  if ((!mem) || mem->size < aligned_size) { // if can probably be simplified to just if(!mem)
    extend(aligned_size);
    free_block = (block_header *) first_node - 1;
    printf("malloc: extend called. found new space at node: %p, hdrp%p\n", first_node, free_block);
  }
  else {
    free_block = mem;
    printf("malloc: no need to call extend- found some space at node: %p, hdrp: %p\n", free_block - 1, free_block);
  }

  // remove node from free list. start by getting ptr to relevant node:
  node* free_list_node = (node *)free_block + 1;

  // set prev node's next to be our next
  if (free_list_node->prev) {
    if(free_list_node->next) free_list_node->prev->next = free_list_node->next;
    else(free_list_node->prev->next) = NULL;
  }

  // set next node's prev to be out prev
  if(free_list_node->next) {
    if(free_list_node->prev) free_list_node->next->prev = free_list_node->prev;
    else free_list_node->next->prev = NULL;
  }
  if(free_list_node == first_node) {
    first_node = NULL;
  }

  // update block header so that it has the correct size and the allocated int is 1
  free_block->size = aligned_size;
  free_block->allocated = 1;

  // return pointer to new block (starting after header)
  printf("mm_malloc: returned successfully\n\n");
  return  (node *)free_block + 1; 
  
}

/*
 * first_fit - helper method to traverse the free list until we find a free block or until we reach
 * the end of the free list (meaning the caller will have to call extend)
*/
block_header* first_fit(size_t size){
  printf("first_fit invoked ------------------------------------\n");
  printf("first_fit: first_node: %p\n", first_node); 
  node *curr = first_node; 
  while((curr!= NULL) && ((block_header *) HDRP(curr))-> size < size){    
    printf("first_fit: traversing this ho! curr: %p\n", curr); 
    curr = curr->next;
  }
  if(curr) printf("first_fit: found a free block at: %p\n", curr); 
  else  printf("first_fit: no free blocks are big enough, need to extend our heap: %p\n", curr); 
  printf("first_fit: returned successfully\n\n");
  if(curr) return((block_header *) HDRP(curr)); 
  return NULL;
}

/*
* mm_free - free block at ptr. No need to check if this is a block that we have access to! Just change the 
* block header to be allocated. TODO: add coalescing to this.
*/
void mm_free(void *ptr)
{
  // TODO: add ptr back to the free list so it can be reused
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