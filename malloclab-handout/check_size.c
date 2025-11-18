#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include "memlib.h"
#include "pagemap.h"

void *curr_block_ptr = NULL;
#define HEADERSIZE 32

typedef struct block_header { 
  size_t size;
  struct block_header* next;
  char allocated;
  struct block_header* prev; 
} block_header; 

int main() {
    printf("Size of block header: %zu bytes\n", sizeof(block_header));

    int size = 4;
    int newsize = ALIGN(size);
    void * new_block = mmap(0, APAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);;
    curr_block_ptr  += newsize + HEADERSIZE;
    
    // make header for new block
    block_header * curr_header = (block_header *) curr_block_ptr;
    curr_header->size = size;
    curr_header->allocated = 1;
    curr_header->prev = prev;

    // return pointer to new block (starting after header)
    return curr_block_ptr;
    
    printf("Address of prev header: %d\n", &prev_header);
    
    printf("Address of curr header: %d\n", &curr_header);
    return 0;
}
