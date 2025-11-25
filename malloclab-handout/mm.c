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

// BLOCK HEADER FOR ALLOCATED MEMORY
typedef struct block_header { 
  size_t packed;
} block_header; 

// BLOCK FOOTER FOR ALLOCATED MEMORY
typedef struct block_footer { 
  size_t packed;
} block_footer; 

// FREE LIST NODE FOR FREE MEMORY
typedef struct node {
  struct node* prev;
  struct node* next; 
} node; 

#define HEADERSIZE sizeof(block_header) // 16 bytes for now
#define FOOTERSIZE sizeof(block_footer) // 16 bytes for now
#define NODESIZE sizeof(node)  // 16 bytes for now

int mapped_pages = 0;
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
  size_t new_size = PAGE_ALIGN(s);  
  block_header * header = mem_map(new_size); 
  
  if (header == NULL) {
    printf("extend: mem_map error\n");
    return;
  }  
  mapped_pages++;
  header->packed = PACK(new_size, 0);
  
  node* bp = (node*)(header + 1);
  block_footer * footer = (block_footer *)FTRP(bp);
  footer->packed = PACK(new_size, 0);

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
  mapped_pages = 0;
  extend(1);
  if (!head) {
    printf("error in mm_init\n");
    return -1;
  }
  return 0;
}
// for now lets ust assum ethat the size of the headers and footers does not include
// the overhead but when we add to memory we scale size up to include overhead

/* Set a block to allocated 
* Update block headers/footers as needed 
* Update free list if applicable 
* Split block if applicable 
*/
static void set_allocated(void *bp, size_t size){   // remember this size includes header / footer
  block_header* header = (block_header *) HDRP(bp);
  block_footer* footer = (block_footer *)FTRP(bp);
  size_t old_size = GET_SIZE(header);
  size_t extra_space = old_size - size;
  header->packed = PACK(size, GET_ALLOC(header));
  delete_node(bp);
  
  // split if possible making sure the space is not unreasonably small
  if(extra_space < (OVERHEAD + NODESIZE + __WORDSIZE)){
    header->packed = PACK(old_size, GET_ALLOC(header));
  }
  else {
    // update allocated block to be smaller
    footer = (block_footer *)FTRP(bp);
    // set up the new free list node
    size_t new_size = extra_space - OVERHEAD;
    node* new_bp = (node*)(((char *)footer) + OVERHEAD);
    block_header* new_header = (block_header*) HDRP(new_bp);
    block_footer* new_footer = (block_footer*) ((char*)new_bp + new_size);
    add_node(new_bp); 
    
    // update block header and footer for the new node
    new_header->packed = PACK(new_size, 0);
    new_footer->packed = PACK(new_size, 0);
  }

  // update the allocated block
  header->packed = PACK(GET_SIZE(header), 1);
  footer->packed = PACK(GET_SIZE(footer), 1);
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
void *mm_malloc(size_t size)
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
  header->packed = PACK(GET_SIZE(header), 0);
  footer->packed = PACK(GET_SIZE(footer), 0);
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
    // printf("total space: %ld\n", header->size);
    // printf("size to allocate: %ld\n", size);
    // printf("extra space: %ld\n", extra_space);
    // printf("Address of header: %p \n", header);
    // printf("Relative address of header: %ld \n", header - header);
    // printf("Relative address of bp: %ld \n", (char*) bp - (char*)header);
    // printf("Relative address of footer: %ld \n", (char*)footer - (char*)header);
    // printf("Relative address of new header: %ld \n", (char*)new_header - (char*)header);
    // printf("Relative address of new bp: %ld \n", (char*)new_bp - (char*)header);
    // printf("Relative address of new footer: %ld \n", (char*)new_footer - (char*)header);
    // printf("Size of free block: %ld\n", new_header->size);
    // printf("Size of allocated block: %ld\n", size);
