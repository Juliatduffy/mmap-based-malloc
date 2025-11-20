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

/// BLOCK HEADER
typedef struct block_header {
    size_t size;                  
    int allocated;                    
    struct block_header *next;    // Next block
    struct block_header *prev;    // Previous  block
} block_header;
int main() {
    printf("Size of block header: %zu bytes\n",  sizeof(block_header));
}
