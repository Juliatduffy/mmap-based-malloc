#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include "mm.h"
#include "memlib.h"
#include "pagemap.h"
#include "fsecs.h"
#include "config.h"

int main() {
    size_t size = 4096;
    printf("Testing malloc with allocating %ld bytes\n", size);
    void* p = mm_malloc(size);
    mm_free(p);
}
