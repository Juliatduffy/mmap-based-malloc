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

#define HDRP(bp) ((char *)(bp) - sizeof(block_header))
#define ALIGNMENT 16 // always use 16-byte alignment
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1)) // rounds up to the nearest multiple of ALIGNMENT
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1)) // rounds up to the nearest multiple of mem_pagesize()


/// BLOCK HEADER FOR ALLOCATED MEMORY TODO: PACK
typedef struct block_header { 
  size_t size;
  char allocated;
} block_header; 

/// FREE LIST NODE
typedef struct node {
  struct node* prev;
  struct node* next; 
} node; 

// TODO make prolog and epilogue for coalescing, make sentinel terminator
// block_header prolog;
// block_header epilogue;

#define HEADERSIZE sizeof(block_header) // 16 bytes 
#define NODESIZE sizeof(node)  // 16 bytes

node *first_node = NULL; // stores head of free linked list, ok to have explicit free list

/*
* Helper for debugging free list
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
HELPERS
- extend (pagealign(size_t))
- memmap (size)
- initialize sentinel
- create empty allocated block
- initalize terminator memmap gives you what you ask for
*/

// // add node delete node
// void * add_node(void * ptr){
// }

// void * delete_node(void * ptr){
// }

/* 
 * mm_init - initialize the malloc package. Not 100% sure what to do here
 * 
 * This method:
 * 1. creates the free list and allocates a free block using mem_map (call extend method)
 * 2. returns -1 on an error, 0 on success
 * 
 */
int mm_init(void)
{
  // free list head = null
  // maybe store # mapped pages
  // extend (1) -> centralizes calling memmap returns pointer to new block extend 1 because minimum is 4096 and we dont need morethan that bc we ewant good util
  // split if possible
  //  allocate all space then turn rest into free block
  //  
    size_t new_size = PAGE_ALIGN(4 * __WORDSIZE);  // init more
    void * new_node = mem_map(new_size); 
    printf("%s %ld %s\n", "initialize_free_list: initialized", new_size, "bytes of memory\n");

    if (new_node == NULL) {
      printf("initialize_free_list: mem_map error\n");
      return;
    }

    // update free list now
    first_node = new_node + HEADERSIZE;
    block_header * header_ptr = (block_header *) new_node;
    header_ptr->size = new_size - NODESIZE - HEADERSIZE;
    header_ptr-> allocated = 0;
    first_node->next = NULL;
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
  printf("malloc: size: %ld\n", size);
  printf("malloc: padded size with header: %ld\n", aligned_size);

  node *curr = first_node;

  // traverse until we find a free block thats big enough TODO pull into a helper
  while(curr != NULL && ((block_header *) HDRP(curr))-> size < aligned_size){
    printf("malloc: traversing this ho\n"); 
    curr = curr->next;
  }
  // if firstfit = size = NULL
  
  if (((block_header *) HDRP(curr))->size < aligned_size) {
      // header_ptr is the last block header in free list and this block is not big enough
      // so we add to it by making this node->next new memory / a new block
      size_t new_size = PAGE_ALIGN(aligned_size); 
      node * new_node = mem_map(new_size); 

      // something went wrong with mem_map
      if (new_node == NULL) {
        printf("malloc: ruh roh");
        return NULL;
      }

      printf("malloc: updating free list node for newly allocated memory at: %p\n", new_node);

      // update free list node for block
      curr->next = new_node;
      new_node->prev =  curr;
      new_node->next = NULL;
      curr = curr-> next;

      printf("malloc: updating block header for newly allocated memory at: %p\n", (block_header *) curr);

      // update block header for block
      block_header * new_header_ptr = (block_header *) curr;
      new_header_ptr-> size = new_size - HEADERSIZE - NODESIZE;
      printf("made it here\n");
      fflush(stdout);
      new_header_ptr-> allocated = 0;
      printf("done with extend\n");
  }

  printf("malloc: updating block header for free list node: %p\n", curr);
  block_header * new_header_ptr = (block_header *) HDRP(curr);
  new_header_ptr->size = aligned_size;
  new_header_ptr-> allocated = 1;

  // return pointer to new block (starting after header and node)
  return curr + HEADERSIZE; 

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
