#include "lib.h"

struct block *get_header(void *addr) {
  // sizeof(void *)*4 - because we have 4 struct bariables above our data pointer
  // and mem_alloc returns block->data pointer
  return (struct block*)(addr - sizeof(void *)*4);
}

size_t alloc_size(size_t size) {
  // we need to allocate memory for block header additionally
  return sizeof(struct block) + size;
}

struct block *request_from_os(size_t size) {
  // get current heap break
  struct block *b = (struct block *) sbrk(0);
  
  if (sbrk(alloc_size(size)) == (void *)-1) {
    return NULL;
  }

  return b;
}

size_t align(size_t n) {
  return (n + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
}

struct block *find_block(size_t size) {
  struct block *cur = heap_start;
  
  for(;(cur != NULL) && (cur->used!=0) && (cur->size<size); cur = cur->next);
  return cur;
}

struct block *split_block(struct block *to_split, size_t size) {
  struct block *new_header = to_split->data + size;
  new_header->size = to_split->size - size;
  new_header->next = to_split->next;
  new_header->prev = to_split;
  new_header->data = to_split->data + size + sizeof(struct block) - sizeof(void *);
  to_split->size = size;
  to_split->next = new_header;

  return to_split;
}

void *mem_alloc(size_t size) {
  size = align(size);

  struct block *b;

  // Try to find block to reuse
  if(b = find_block(size)) {
    // It's impossible to split block if it's data segment 
    // can't contain new block of minimum aligned size
    if(b->size >= (size + sizeof(struct block)) + sizeof(void *)) {
      b = split_block(b, size);
      b->used = 1;
    }
    return b->data;
  }
  
  // if not found, request new from OS
  b = request_from_os(size);
  b->size = size;
  b->used = 1;
  b->data = (void *)((void *) (b) + sizeof(void *)*4);

  // Init heap
  if(heap_start == NULL) {
    heap_start = b;
  }
  // Chain blocks
  if(top == NULL) {
    top = b;
  } else {
    top->next = b;
    b->prev = top;
  }
  top = b;

  return b->data;
}

void *mem_realloc(void *addr, size_t size) {
}

void mem_free(void *addr) {
  struct block *b = get_header(addr);
  struct block *to_merge;
  b->used = 0;
  if (b->next && b->next->used == 0) {
    to_merge = b->next;
    b->size = b->size + to_merge->size;
  }
  if(b->prev && b->prev->used == 0) {
    to_merge = b->prev;
    to_merge->size = to_merge->size + b->size;
  }
}
