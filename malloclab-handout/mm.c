/*
 * mm-naive.c
 * author: Julia Duffy and CS4400 at the University of Utah
 * last edited: 11-22-2025
 * current implementation: explicit free list with no coalescing or splitting (10/100).
 * next implementation: same thing but with freeing.
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

// BLOCK HEADER FOR ALLOCATED MEMORY
typedef struct block_header { 
  size_t size;
  char allocated;
} block_header; 

// FREE LIST NODE FOR FREE MEMORY
typedef struct node {
  struct node* prev;
  struct node* next; 
} node; 

#define HEADERSIZE sizeof(block_header) // 16 bytes for now
#define NODESIZE sizeof(node)  // 16 bytes for now

node *head = NULL; 

/*
* Helper functions
*/
void print_free_list_summary(void);
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void extend(size_t s);
void add_node(node* ptr);
void delete_node(node* ptr);
node* first_fit(size_t size);

/*
 * delete node from the free list 
*/
void delete_node(node* ptr){
  if(!head || !ptr) {
    printf("error in delete_node\n");
    return;
  } 
  if(ptr == head) {
    head = head-> next;
  }
  if (ptr->prev) {
    ptr->prev->next = ptr->next;
  }
  if(ptr->next) {
    ptr->next->prev = ptr->prev;
  }
}

/*
 * add node to the free list (at the head for now) 
*/
void add_node(node *ptr) {
    if (!ptr) {
      printf("error in add_node\n");
      return;
    }
    ptr->next = head;
    ptr->prev = NULL;

    if (head) {
        head->prev = ptr;
    }
    head = ptr;
}

/*
* extend - extends our "heap" size
* 
* 1. align the given size to me a mutiple of 4096
* 2. allocate at least 4096 bytes of heap memory using mem_map
* TODO
* 3. update the free list, inserting new memory as the head of the list
*/
void extend(size_t s) {
  size_t new_size = PAGE_ALIGN(s);  
  block_header * new_block = mem_map(new_size); 

  if (new_block == NULL) {
    printf("extend: mem_map error\n");
    return;
  }  

  new_block->size = new_size;
  new_block->allocated = 0;

  add_node((node*)(new_block + 1));  
}


/* 
 * mm_init - initialize the malloc package
 * 
 * 1. reset the free list 
 * 2. call the extend method
 * 3. return -1 on an error, 0 on success
 */
int mm_init(void)
{
  head = NULL;
  extend(1);
  if (!head) {
    printf("error in mm_init\n");
    return -1;
  }
  return 0;
}

/* 
*  mm_malloc - Allocate a block in the free list,
*  grabbing a new page if necessary.
*  
*  1. align/pad the given size 
*  2. try to find a free block in the free list
*  3. call extend if necessary, which extends our available heap memory
*     by calling mem_map, and adds a new node to the head of our free list
*  4. create a new block header for the memory we are allocating
*  5. return the pointer to the new memory (after the block header)
*/
void *mm_malloc(size_t size)
{
  size += HEADERSIZE; 
  size_t aligned_size = ALIGN(size);
  
  // try to find a free block
  node* free_node = first_fit(aligned_size);

  // if there are no free blocks, extend
  if (!free_node) {
    extend(aligned_size);
    free_node = head;
  }

  // add block metadata
  block_header* free_block = (block_header *) HDRP(free_node);
  free_block->size = aligned_size;
  free_block->allocated = 1;

  // delete node from the free list
  delete_node(free_node);

  // return pointer to newly allocated block
  return free_node; 
  
}

/*
 * first_fit - helper method to traverse the free list until we find a free block or until we reach
 * the end of the free list (in which case the caller will have to call extend)
*/
node* first_fit(size_t size){
  node *curr = head; 
  while((curr!= NULL) && ((block_header *) HDRP(curr))-> size < size){    
    curr = curr->next;
  }
  return curr;
}

/*
* mm_free - free block at ptr. 
*/
void mm_free(void *ptr)
{
  // TODO: add ptr back to the free list so it can be reused
  // TODO coalesce
  // unmap page if it's all free and there are a decent amount of pages
}

/*
* Helper to print all free list elements
*/
void print_free_list_summary(void){
  printf("free list summary:\n");
  printf("head: %p\n", head);
  node* ptr = head;
   while( ptr ) {
      printf( "block addr: %p\n", ptr);
      ptr = ptr->next;
   }
}

/*
Notes: 
- we don't need to check that free is being called on memory that we have acess to 
  for this assignment we can just assume we can free whatever pointer is passed in.

- The reason why we don't want to get more heap memory at once is because 
  if we ran a test where we were just freeing and allocating the same 8 bytes
  then we would haave terrible utilization.

- Breaking things up into helpers is really useful for this assignemnt

- Don't call mem_map too oftem because it is slow

- Eventually it will be a good idea to map pages so that if we have a decent amount of 
  pages of memory and we have a completely freed page, we can remove that entire page 
  from the free list. we can do this by keeping track of mapped pages

- extend should eventually implement splitting

*/


// TODO: 
// - implement free without coalescing yet
// - pack header to 8 bytes with pack function
// - make 8 byte block footer struct
// - make prolog and epilogue for coalescing within each heap page
// - make sentinel and terminator blocks (for beginning and end of heap)
// - implement splitting
// - implement coalescing
// - implement paging? 
// - 
