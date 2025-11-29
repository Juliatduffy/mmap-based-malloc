/*
 * mm-naive.c
 * author: Julia Duffy and CS4400 at the University of Utah
 * last edited: 11-28-2025
 * current implementation: splitting with packed headers and prologue and epilogue blocks (24/100).
 * next implementation: same thing but with removing unmapped pages, then with coalescing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"
#include "macros.c"

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1)) 

// BLOCK HEADER AND FOOTER FOR ALLOCATED MEMORY
typedef size_t block_header, block_footer;

// FREE LIST NODE FOR FREE MEMORY
typedef struct node {
  struct node* prev;
  struct node* next; 
} node; 

#define HEADERSIZE (sizeof(block_header)) // 8 bytes
#define FOOTERSIZE (sizeof(block_footer)) // 8 bytes
#define OVERHEAD (sizeof(block_header)+sizeof(block_footer)) // 16 bytes
#define EXTEND_OVERHEAD (4 * sizeof(block_header)) // 32 bytes
#define NODESIZE (sizeof(node))  // 16 bytes 

node *head = NULL; 
int mapped_pages_count = 0;

// Main functions
int mm_init(void);
void* mm_malloc(size_t size);
void mm_free(void *ptr);

// Helper functions
static inline void extend(size_t s);
static inline void add_node(node* ptr);
static inline void delete_node(node* ptr);
static inline node* first_fit(size_t size);
static inline void* coalesce(void *bp);

/*
 * delete_node - deletes node from the free list 
*/
static inline void delete_node(node* ptr){
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
 * add_node - add a new node to the free list at the head
*/
static inline void add_node(node *ptr) {
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
*/
static inline void extend(size_t s) {
  size_t size = PAGE_ALIGN(2 * s); 
  block_header * new_page = (block_header*) mem_map(size);

  PUT(new_page, 0);  // alignment
  PUT(new_page + 1, PACK(OVERHEAD, 1));  // prologue header
  PUT(new_page + 2, PACK(OVERHEAD, 1));   // prologue footer
  PUT(new_page + 3, PACK(size - EXTEND_OVERHEAD, 0));   // block header
  node* bp = (node*)(new_page + 4);  // payload pointer
  PUT(FTRP(bp), PACK(size - EXTEND_OVERHEAD, 0));  // block footer
  PUT(FTRP(bp) + FOOTERSIZE, PACK(0, 1));  // epilogue header
  add_node(bp);
  mapped_pages_count++;
}

/* 
 * mm_init - initialize the malloc package
 */
int mm_init(void)
{
  head = NULL;
  mapped_pages_count = 0;
  extend(1);
  if (!head) {
    printf("error in mm_init\n");
    return -1;
  }
  return 0;
}

/* 
* set_allocated - sets block at given bp to allocated 
*/
static inline void set_allocated(void *bp, size_t size){
  size_t old_size = GET_SIZE(HDRP(bp));
  int extra_space = old_size - size;
  delete_node((node*)bp);

  // no split
  if((extra_space < OVERHEAD + NODESIZE) || extra_space < 0){ 
    PUT(HDRP(bp), PACK(old_size, 1));
    PUT(FTRP(bp), PACK(old_size, 1));
  }
  // split
  else {
    PUT(HDRP(bp), PACK(size, 1)); 
    PUT(FTRP(bp), PACK(size, 1));

    // set up the new free list node
    node* new_bp = (node*) NEXT_BLKP(bp); 
    add_node(new_bp); 

    // update block header and footer for the new node
    PUT(HDRP(new_bp), PACK(extra_space, 0));
    PUT(FTRP(new_bp), PACK(extra_space, 0));
  }
}

/* 
*  mm_malloc - allocate a block in the free list, grabbing a new page if necessary.
*/
void* mm_malloc(size_t size)
{
  size += OVERHEAD; 
  size_t aligned_size = ALIGN(size);

  // try to find a free block
  node* bp = first_fit(aligned_size);

  // if there are no free blocks, extend
  if (!bp) {
    aligned_size += EXTEND_OVERHEAD;
    extend(aligned_size);
    bp = head;
  }
  
  // set free block to allocated, split if possible
  set_allocated(bp, aligned_size);
  return bp; 
}

/*
* first_fit - helper method to traverse the free list until we 
* find a free block or until we reach the end of the free list
*/
static inline node* first_fit(size_t size){
  node *curr = head; 
  while((curr!= NULL) && (GET_SIZE(HDRP(curr))) < size){ 
    curr = curr->next;
  }
  return curr;
}

/*
* mm_free - free block at ptr
*/
void mm_free(void *bp)
{
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp),PACK(size,0));
  PUT(FTRP(bp),PACK(size,0));
  bp = coalesce(bp);
  if(mapped_pages_count > 2 && GET_SIZE((bp)) == OVERHEAD){
    mapped_pages_count--;
  }
  else {
    add_node(bp);
  }
}

static inline void* coalesce(void* bp)
{
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if(prev_alloc && next_alloc){
    return bp;
  }
  
  if(prev_alloc && !next_alloc){
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    // delete node that is getting absolved
    delete_node((node*)NEXT_BLKP(bp));
    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
  }
  
  else if(!prev_alloc && next_alloc){ 
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    delete_node((node*)PREV_BLKP(bp));
    PUT(FTRP(bp),PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
    bp = PREV_BLKP(bp);
  }

  else if(!prev_alloc && !next_alloc){ 
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    delete_node((node*)NEXT_BLKP(bp));
    delete_node((node*)PREV_BLKP(bp));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
    bp = PREV_BLKP(bp);
  }
  return bp;
}