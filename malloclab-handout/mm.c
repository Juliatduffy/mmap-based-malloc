/*
 * mm-naive.c
 * author: Julia Duffy and CS4400 at the University of Utah
 * last edited: 11-22-2025
 * current implementation: explicit free list with no coalescing or splitting (10/100).
 * next implementation: same thing but with block footers and splitting.
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
typedef size_t block_header;
typedef size_t block_footer;

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

/*
* Helper functions
*/
void print_free_list_summary(void);
int mm_init(void);
void* mm_malloc(size_t size);
void mm_free(void *ptr);
void extend(size_t s);
void add_node(node* ptr);
void delete_node(node* ptr);
node* first_fit(size_t size);

/*
 * delete_node - deletes node from the free list 
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
 * add_node - add a new node to the free list at the head
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
*/
void extend(size_t s) {
  size_t size = PAGE_ALIGN(s); 
  block_header * new_page = mem_map(size);

  PUT(new_page, 0);  // alignment
  PUT(new_page + 1, PACK(OVERHEAD, 1));  // prologue header
  PUT(new_page + 2, PACK(OVERHEAD, 1));   // prologue footer
  PUT(new_page + 3, PACK(size - EXTEND_OVERHEAD, 0));   // block header
  node* bp = (node*)(new_page + 4);  // payload pointer
  
  PUT(FTRP(bp), PACK(size - EXTEND_OVERHEAD, 0));  // block footer
  PUT(FTRP(bp) + FOOTERSIZE, PACK(0, 1));  // epilogue header
  
  add_node(bp);
  // block_header* header =  (block_header*) HDRP(bp);
  // block_footer* footer =  (block_footer*) FTRP(bp);
  // printf("\nextend:\n");
  // printf("Size of new block: %ld\n", size);
  // printf("Header size: %ld\n", GET_SIZE(header));
  // printf("Address of header: %p \n", header);
  // printf("Address of bp: %p \n",bp );
  // printf("Address of footer: %p \n", footer);
  // printf("Relative address of footer: %ld \n", (char*)footer - (char*)header); 
  // print_free_list_summary();

}


/* 
 * mm_init - initialize the malloc package
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
* set_allocated - sets block at given bp to allocated 
*/
static void set_allocated(void *bp, size_t size){
  //printf("set allocated size: %ld\n", size);
  size_t old_size = GET_SIZE(HDRP(bp));
  int extra_space = old_size - size;
  delete_node(bp);

  // no split
  if((extra_space <= OVERHEAD + NODESIZE) || extra_space < 0){ 
    PUT(HDRP(bp), PACK(old_size, 1));
    PUT(FTRP(bp), PACK(old_size, 1));
    //printf("no split\n");
  }
  // split
  else {
    //printf("splitting this ho\n");
    PUT(HDRP(bp), PACK(size, 1)); 
    PUT(FTRP(bp), PACK(size, 1));

    // set up the new free list node
    size_t new_size = extra_space;
    node* new_bp = (node*) NEXT_BLKP(bp); 
    add_node(new_bp); 

    // update block header and footer for the new node
    PUT(HDRP(new_bp), PACK(new_size, 0));
    PUT(FTRP(new_bp), PACK(new_size, 0));

    // block_header* header =  (block_header*) HDRP(bp);
    // block_footer* footer =  (block_footer*)FTRP(bp);
    // block_header* new_header =  (block_header*)HDRP(new_bp);
    // block_footer* new_footer = (block_footer*)FTRP(new_bp);
    // printf("extra space: %d\n", extra_space);
    // printf("Address of header: %p \n", header);
    // printf("Address of bp: %p \n",bp );
    // printf("Old header: %p \n", header);
    // printf("Old footer: %p \n", footer);
    // printf("Relative address of new header: %ld \n", (char*)new_header - (char*)new_header);
    // printf("Relative address of new bp: %ld \n", (char*)new_bp - (char*)new_header);
    // printf("new header: %p, new footer: %p\n", new_header, new_footer);
    // printf("Relative address of new footer: %ld \n", (char*)new_footer - (char*)new_header);
    // printf("Size of free block: %ld\n", GET_SIZE(new_header));
    // printf("Size of allocated block: %ld\n", size);
    // print_free_list_summary();
  }
}

/* 
*  mm_malloc - allocate a block in the free list,
*  grabbing a new page if necessary.
*/
void* mm_malloc(size_t size)
{
  //printf("\nmalloc\n");
  size += OVERHEAD; 
  size_t aligned_size = ALIGN(size);
  //printf("malloc aligned size: %ld\n", aligned_size);

 
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
 * first_fit - helper method to traverse the free list until we find a free block or until we reach
 * the end of the free list (in which case the caller will have to call extend)
*/
node* first_fit(size_t size){
  node *curr = head; 
  while((curr!= NULL) && (GET_SIZE(HDRP(curr))) <= size){    // FIXME: <= here? or just <
    curr = curr->next;
  }
  return curr;
}

/*
* mm_free - free block at ptr. 
*/
void mm_free(void *ptr)
{
  add_node(ptr);
  block_header* header = (block_header *)HDRP(ptr);
  block_footer* footer = (block_footer *)FTRP(ptr);
  PUT(header, PACK(GET_SIZE(header), 0));
  PUT(footer, PACK(GET_SIZE(footer), 0));
  // TODO: coalesce
}

/*
* print_free_list_summary - helper to print all free list elements
*/
void print_free_list_summary(void){
  printf("free list summary:\n");
  printf("head: %p\n", head);
  node* ptr = head;
   while( ptr ) {
      printf( "block hdrp: %p\n", HDRP(ptr));
      printf( "block ftrp: %p\n", FTRP(ptr));
      ptr = ptr->next;
   }
}

/*
Notes: 
- we don't need to check that free is being called on memory that we have acess to 
  for this assignment we can just assume we can free whatever pointer is passed in.

- Eventually it will be a good idea to map pages so that if we have a decent amount of 
  pages of memory and we have a completely freed page, we can remove that entire page 
  from the free list. we can do this by keeping track of mapped pages

- make sentinel and terminator blocks (for beginning and end of heap)

- implement coalescing

- could add this to make allocated size even but idk why exactly you would do this:
  if (size % 2) {
        size += 1;
  }

*/




    
// uncomment to debug
// printf("total space: %ld\n", GET_SIZE(header));
// printf("size to allocate: %ld\n", size);
// printf("extra space: %ld\n", extra_space);
// printf("Address of header: %p \n", header);
// printf("Relative address of header: %ld \n", header - header);
// printf("Relative address of bp: %ld \n", (char*) bp - (char*)header);
// printf("Address of bp: %p \n",bp );
// printf("Relative address of footer: %ld \n", (char*)footer - (char*)header);
// printf("Relative address of new header: %ld \n", (char*)new_header - (char*)header);
// printf("Relative address of new bp: %ld \n", (char*)new_bp - (char*)header);
// printf("Relative address of new footer: %ld \n", (char*)new_footer - (char*)header);
// printf("Size of free block: %ld\n", GET_SIZE(new_header));
// printf("Size of allocated block: %ld\n", size);

// printf("new block: %p\n", new_page);
// printf("\nextend: size: %ld\n", size);
// printf("prologue header: %p\n", new_page+1);
// printf("prologue footer: %p\n", new_page+2);
// printf("block header: %p\n", new_page+3);
// printf("bp: %p\n", new_page+4);
// printf("block footer: %p\n", ((block_header*)FTRP(bp))-1);
// printf("epilogue: %p\n", ((char*)new_page + size - 8));
