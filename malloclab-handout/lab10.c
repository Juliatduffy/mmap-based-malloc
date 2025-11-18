#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

static char* read_line();

static void init_heap();

static void add_string(char* s);
static void remove_string(char* s);
static void show_strings();

int main() {
  init_heap();

  printf("Usage:\n");
  printf(" A blank line exits.\n");
  printf(" Enter \"?\" to see the current list.\n");
  printf(" Any other line that doesn't start \"-\" and adds to the list.\n");
  printf(" A line that starts \"-\" removes the rest of the line from the list.\n");

  while(1) {
    char* s = read_line();

    if(!strcmp(s, "?"))
      show_strings();
    else if(s[0] == '-')
      remove_string(s+1);
    else
      add_string(s);
  }
}

static char* read_line() {
  size_t sz = 32, i = 0;
  char* s = malloc(sz+1);

  while(1) {
    int c = fgetc(stdin);
    if((c == EOF) || (c == '\n')) {
      if(i == 0)
        exit(0);
      s[i] = 0;
      return s;
    }

    if(i == sz) {
      char* new_s = malloc(2*sz+1);
      memcpy(new_s, s, i);
      free(s);
      s = new_s;
    }
    s[i++] = c;
  }
  
  return s;
}

/* ************************************************************ */
/*  Linked-list allocator                                       */
/* ************************************************************ */

#define LINK_PTRS 3
#define DATA(l) (((void**)(l))[0])
#define PREV(l) (((void**)(l))[1])
#define NEXT(l) (((void**)(l))[2])

static void* hd = NULL;
static void* tl = NULL;

static int heap_size = 4096;
static void* heap;
static void* alloc_p;

static void init_heap() {
  heap = mmap(NULL, heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  memset(heap, 0, heap_size);
  alloc_p = heap;
}

/* CHANGE: added check function */
void check() {
  void* l;
  void* prev;

  /* Check linking of doubly linked list -- consistency properties (part 1) */
  prev = NULL;
  for(l = hd; l != NULL; l = NEXT(l)) {
    if(PREV(l) != prev)
      printf("wrong prev\n");
    prev = l;
  }

  if(prev != tl)
    printf("wrong tl\n");

  /* Check that allocated links fill the expected region (part 2)*/
  for(l = heap; l < alloc_p; l += LINK_PTRS*sizeof(void*)) {
    if(DATA(l) == NULL)
      printf("wasted space\n");
  }
}

static void add_string(char* s) {
  /* CHANGE: void* instead of void** (void** results in gap in allocation) */
  void* l;
  
  for(l = hd; l != NULL; l = NEXT(l)) {
    if(!strcmp(s, DATA(l)))
      return;
  }

  l = alloc_p;
  alloc_p = l + (LINK_PTRS * sizeof(void*));
  if(alloc_p > (heap + heap_size)) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }

  DATA(l) = s;
  /* CHANGE: repair #1 -- set prev link */
  PREV(l) = tl;
  if(tl)
    NEXT(tl) = l;
  else
    hd = l;
  tl = l;
  NEXT(l) = NULL;

  /* CHANGE: added check after every add */
  check();
}

static void remove_string(char* s) {
  void* l;
  
  for(l = hd; l != NULL; l = NEXT(l)) {
    if(!strcmp(s, DATA(l))) {
      if(PREV(l))
        NEXT(PREV(l)) = NEXT(l);
      /* CHANGE: repair #2 -- handle remove at head of list */
      else
        hd = NEXT(l);
      if(NEXT(l))
        PREV(NEXT(l)) = PREV(l);
      /* CHANGE: repair #3 -- handle remove at tail of list */
      else
	tl = PREV(l);
      break;
    }
  }

  /* CHANGE: added check after every remove */
  check();
}

static void show_strings() {
  void* l;

  for(l = hd; l != NULL; l = NEXT(l)) {
    printf("%s\n", DATA(l));
  }
}
