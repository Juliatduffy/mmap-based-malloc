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

/// BLOCK HEADER FOR ALLOCATED MEMORY
typedef struct block_header { 
  size_t size;
  char allocated;
} block_header; 

/// FREE LIST NODE
typedef struct node {
  size_t size; // size of this block
  void * next; 
  void * prev;  
  int filler;
} node; 


int main() {
    printf("Size of node: %zu bytes. Size of block header: %zu bytes\n", sizeof(node), sizeof(block_header));
}
