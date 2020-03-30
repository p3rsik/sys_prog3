#include "lib.h"
#include "stdio.h"
#include "assert.h"

int main() {
  printf("sizeof(block) = %d\n", sizeof(struct block));

  printf("Allocating...\n");
  void *p1 = mem_alloc(8);
  void *p2 = mem_alloc(228);
  printf("Allocated: %p and %p\n\n", p1, p2);
  struct block *h1;
  if(p1) {
    h1 = get_header(p1);
    printf("Got header1: %p\n", h1);
    printf("Size1 = %d, aligned size = %d\n", 8, h1->size);
  }
  if(p2) {
    struct block *h2 = get_header(p2);
    printf("Got header2: %p\n", h2);
    printf("Size2 = %d, aligned size = %d\n", 228, h2->size);
    printf("Freeing memory for the second block...\n");
    mem_free(p2);
    assert(is_used(h2) == 0);
    printf("Allocating 20 more bytes...\n");
    void *p3 = mem_alloc(20);
    printf("Allocated: %p\n", p3);
    if(p1 && p3) {
      struct block *h3 = get_header(p3);
      assert(h3->prev == h1);
    }
  }
  
  printf("\n");
  // Let's see memory state after all the allocation, deallocation and splitting
  mem_dump();
  
  return 0;
}
