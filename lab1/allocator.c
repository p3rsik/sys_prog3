#include "lib.h"
#include "stdio.h"
#include "assert.h"

int main() {
  printf("sizeof(block) = %d\n", sizeof(struct block));

  printf("Allocating...\n");
  void *p1 = mem_alloc(8);
  if(!p1)
    return 1;

  void *p2 = mem_alloc(228);
  if(!p2)
    return 1;

  printf("Allocated: %p and %p\n\n", p1, p2);
  block *h1 = get_header(p1);
  printf("Got header1: %p\n", h1);
  printf("Size1 = %d, aligned size = %d\n", 8, get_size(h1));

  block *h2 = get_header(p2);
  printf("Got header2: %p\n", h2);
  printf("Size2 = %d, aligned size = %d\n", 228, get_size(h2));

  // Test block linking
  assert(get_next(h1) == h2);
  assert(get_prev(h2) == h1);

  printf("Freeing memory for the second block...\n");
  mem_free(p2);

  // Test used bit flag setting
  assert(is_used(h2) == 0);
  
  printf("Allocating 20 more bytes...\n");
  void *p3 = mem_alloc(20);
  if(!p3)
    return 1;

  printf("Allocated: %p\n", p3);
  block *h3 = get_header(p3);
  printf("Got header3: %p\n", h3);
  printf("Size3 = %d, aligned size = %d\n", 20, get_size(h3));

  // Test block linking
  assert(get_prev(h3) == h1);
  assert(has_next(h3) == 1);
  assert(get_prev(get_next(h3)) == h3);
  
  printf("============================\n");
  mem_dump();
  printf("============================\n");
  
  printf("Freeing memory for the first block...\n");
  mem_free(p1);

  assert(is_used(h1) == 0);

  printf("Allocating 20 more bytes...\n");
  void *p4 = mem_alloc(20);
  if(!p4)
    return 1;
  
  printf("Allocated: %p\n", p4);
  block *h4 = get_header(p4);
  printf("Got header4: %p\n", h4);
  printf("Size4 = %d, aligned size = %d\n", 20, get_size(h4));

  printf("============================\n");
  mem_dump();
  printf("============================\n");

  printf("Freeing memory for the first 20-byte block...\n");
  mem_free(p3);
  
  assert(get_next(h1) == h4);
  assert(get_prev(h4) == h1);

  printf("\n");
  // Let's see memory state after all the allocation, deallocation, splitting and merging
  printf("============================\n");
  mem_dump();
  printf("============================\n");
  
  return 0;
}
