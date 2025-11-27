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

#define HEADERSIZE sizeof(block_header) // 8 bytes
#define FOOTERSIZE sizeof(block_footer) // 8 bytes
#define OVERHEAD (sizeof(block_header)+sizeof(block_footer)) // 16 bytes
#define EXTEND_OVERHEAD (4 * sizeof(block_header)) // 32 bytes
#define NODESIZE sizeof(node)  // 16 bytes 

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
* 
* 1. align the given size to me a mutiple of 4096
* 2. allocate at least 4096 bytes of heap memory using mem_map
* 3. if size < mem_map then allocate size amount of bytes and add the rest
*    of the new block to the free list 
* 4. update the free list, inserting new memory as the head of the list
*/
void extend(size_t s) {
  size_t size = PAGE_ALIGN(s); 
  block_header * new_page = mem_map(size);

  PUT(new_page, 0);  // alignment
  PUT(new_page + 1, PACK(OVERHEAD, 1));  // prologue header
  PUT(new_page + 2,  PACK(OVERHEAD, 1));   // prologue footer
  PUT(new_page + 3,  PACK(size - EXTEND_OVERHEAD, 0));   // block header
  node* bp = (node*)(new_page + 4);  // payload pointer
  PUT(FTRP(bp), PACK(size - EXTEND_OVERHEAD, 0));  // block footer
  PUT(FTRP(bp) + FOOTERSIZE, PACK(0, 1));  // epilogue header
  
  add_node(bp);
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

/* Set a block to allocated 
*  1. Update block headers/footers as needed 
*  2. Update free list if applicable 
*  3. Split block if applicable 
*/
static void set_allocated(void *bp, size_t size){
  size_t old_size = GET_SIZE(HDRP(bp));
  size_t extra_space = old_size - size;
  
  delete_node(bp);

  // no split
  if(extra_space < (OVERHEAD + NODESIZE + 2 * __WORDSIZE)){ 
    PUT(HDRP(bp), PACK(old_size, 1));
    PUT(FTRP(bp), PACK(old_size, 1));
  }
  // split
  else {
    // update allocated block
    PUT(HDRP(bp), PACK(size, 1)); 
    PUT(FTRP(bp), PACK(size, 1));

    // set up the new free list node
    size_t new_size = extra_space;
    node* new_bp = (node*) NEXT_BLKP(bp); 
    
    add_node(new_bp); 

    // update block header and footer for the new node
    PUT(HDRP(new_bp), PACK(new_size, 0));
    PUT(FTRP(new_bp), PACK(new_size, 0));
  }
}

/* 
*  mm_malloc - allocate a block in the free list,
*  grabbing a new page if necessary.
*  
*  1. align/pad the given size 
*  2. try to find a free block in the free list
*  3. call extend if necessary, which extends our available heap memory
*     by calling mem_map, and adds a new node to the head of our free list
*  4. create a new block header for the memory we are allocating
*  5. return the pointer to the new memory (after the block header)
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
  set_allocated(bp, aligned_size);
  return bp; 
}

/*
 * first_fit - helper method to traverse the free list until we find a free block or until we reach
 * the end of the free list (in which case the caller will have to call extend)
*/
node* first_fit(size_t size){
  node *curr = head; 
  while((curr!= NULL) && (GET_SIZE((block_header *) HDRP(curr))) < size){    
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