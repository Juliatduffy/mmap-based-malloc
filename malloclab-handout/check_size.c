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
    printf("Size of node: %zu bytes. Size of block header: %zu\n", sizeof(node), sizeof(block_header));
}
