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

#define PACK(size, alloc) ((size) | (alloc))

// BLOCK HEADER FOR ALLOCATED MEMORY TODO: PACK
typedef struct block_header { 
  size_t size;
  char allocated;
} block_header; 

// FREE LIST NODE
typedef struct node {
  struct node* prev;
  struct node* next; 
} node; 

int main() {
  size_t packed = PACK(2, 1);
  printf("Size of node: %zu bytes. Size of block header: %zu\n", sizeof(node), sizeof(packed));
}
