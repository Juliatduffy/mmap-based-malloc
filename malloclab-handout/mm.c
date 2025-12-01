/*
 * mm-naive.c
 * author: Julia Duffy and CS4400 at the University of Utah
 * last edited: 11-28-2025
 * current implementation: explicit free list, splitting, freeing, coalescing, 
 * first fit, unmapping of unused pages, smart chunk mapping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

// BLOCK HEADER AND FOOTER FOR ALLOCATED MEMORY
typedef size_t block_header, block_footer;

// FREE LIST NODE FOR FREE MEMORY
typedef struct node {
  struct node* prev;
  struct node* next; 
} node; 

// PROVIDED MACROS
#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1)) 
#define HDRP(bp) ((char *)(bp) - sizeof(block_header))
#define FTRP(bp) ((char *)(bp)+GET_SIZE(HDRP(bp))-OVERHEAD)
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-OVERHEAD))
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))
#define PACK(size, alloc) ((size) | (alloc))
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_SIZE(p) (GET(p) & ~0xF)
#define OVERHEAD (sizeof(block_header)+sizeof(block_footer)) // 16 bytes

// MY MACROS
#define HEADER_SIZE (sizeof(block_header)) // 8 bytes
#define FOOTER_SIZE (sizeof(block_footer)) // 8 bytes
#define NODE_SIZE (sizeof(node))  // 16 bytes 
#define EXTEND_OVERHEAD (4 * sizeof(block_header)) // 32 bytes
#define PAGE_PTR(bp) ((char *)(bp) - EXTEND_OVERHEAD) 
#define PAGE_SIZE(bp) (GET(PAGE_PTR(bp))) // varies bc of doubling

// FREE LIST HEAD
node *head = NULL;

// NUMBER OF MAPPED PAGES
int mapped_pages_count = 0;

// MAIN FUNCTIONS
int mm_init(void);
void* mm_malloc(size_t size);
void mm_free(void *ptr);

// HELPER FUNCTIONS
static inline void extend(size_t s);
static inline void add_node(node* ptr);
static inline void delete_node(node* ptr);
static inline node* first_fit(size_t size);
static inline void* coalesce(void *bp);
static inline int is_first_block(void *ptr);
static inline int page_is_free(void *bp);

/*
 * delete_node - deletes node from the free list 
*/
static inline void delete_node(node* ptr){
  if(ptr == head) {
    head = head-> next;
  }
  if(ptr->prev) {
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
  size_t size = PAGE_ALIGN(mapped_pages_count * s);
  block_header * new_page = (block_header*) mem_map(size);
  
  PUT(new_page, 0);  // alignment
  PUT(new_page + 1, PACK(OVERHEAD, 1));  // prologue header
  PUT(new_page + 2, PACK(OVERHEAD, 1));   // prologue footer
  PUT(new_page + 3, PACK(size - EXTEND_OVERHEAD, 0));   // block header
  node* bp = (node*)(new_page + 4);  // payload pointer
  PUT(FTRP(bp), PACK(size - EXTEND_OVERHEAD, 0));  // block footer
  PUT(FTRP(bp) + FOOTER_SIZE, PACK(0, 1));  // epilogue header
  add_node(bp);
  mapped_pages_count++;
}

/* 
 * mm_init - initialize the malloc package
 */
int mm_init(void)
{
  head = NULL;
  mapped_pages_count = 1;
  extend(1);
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
  if((extra_space < OVERHEAD + NODE_SIZE) || extra_space < 0){ 
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
* is_first_block - check to see if this is the first blockin heap page
*/
static inline int is_first_block(void *bp) {
  block_header * prologue_ptr = (block_header *)((char*)bp - OVERHEAD);
  size_t size = GET_SIZE(prologue_ptr);
  int alloc = GET_ALLOC(prologue_ptr);
  return (size == OVERHEAD && alloc == 1);
}

/*
* page_is_free - check to see if this page can be unmapped ie all blocks in it are unallocated
*/
static inline int page_is_free(void *bp) {
  return 0;
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

  // unmap page if it is empty and we have enough pages
  if(mapped_pages_count > 2 && is_first_block(bp) && page_is_free(PAGE_PTR(bp))) {
    // TODO: unmap that shit
  } 
  // otherwise add the node back to the free list  
  else {
    add_node(bp);
  }
}

static inline void* coalesce(void* bp)
{
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));
  
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