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

typedef struct block_header { 
  size_t size;
  char allocated;
} block_header; 


int main() {
    printf("Size of node: %zu bytes\n", sizeof(block_header));
}
