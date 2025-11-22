/*
 * mm-naive.c
 * author: Julia Duffy and CS4400 at the University of Utah
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

node *head = NULL; 

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
* Extend: extends our "heap" size
* 1. align the given size to me a mutiple of 4096
* 2. allocate at least 4096 bytes of heap memory using mem_map
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
  head = NULL;
  extend(1);
  if (!head) {
    printf("error in mm_init\n");
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

  //print_free_list_summary();

  // return pointer to new block (starting after header)
  return  free_node; 
  
}

/*
 * first_fit - helper method to traverse the free list until we find a free block or until we reach
 * the end of the free list (meaning the caller will have to call extend)
*/
node* first_fit(size_t size){
  //printf("first_fit: head: %p\n", head); 
  node *curr = head; 
  while((curr!= NULL) && ((block_header *) HDRP(curr))-> size < size){    
    curr = curr->next;
  }
  //printf("first_fit returned: %p\n", curr);
  return curr;
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