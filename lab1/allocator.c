#include "lib.h"
#include "stdio.h"
#include "assert.h"

int main() {
  printf("sizeof(block) = %d\n", sizeof(struct block));

  printf("Allocating...\n");
  void *p1 = mem_alloc(3);
  printf("Allocated: %p\n\n", p1);
  struct block *h1 = get_header(p1);
  printf("Got header: %p\n", h1);
  printf("Size = %d, aligned size = %d\n", 3, h1->size);
  assert(h1->size == sizeof(void *));
  printf("Freeing memory...\n");
  mem_free(p1);
  assert(h1->used == 0);
  
  return 0;
}
