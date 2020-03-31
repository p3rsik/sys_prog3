#include "struct.h"

size_t get_size(block *b) {
  return b->size - (b->size % sizeof(void *));
}

unsigned char is_used(block *b) {
  return b->size % sizeof(void *);
}

unsigned char has_next(block *b) {
  return (unsigned char) ((long) (b->prev) % sizeof(void *));
}

block *get_next(block *b) {
  if(has_next(b)) {
    return (block *) ((char *) (b->data) + get_size(b));
  }
  return NULL;
}

void set_next(block *b) {
  b->prev = (block *) ((long) (b->prev) + 1);
}

void unset_next(block *b) {
  b->prev = (block *) ((long) (b->prev) - 1);
}

block *get_prev(block *b) {
  long t = (long) (b->prev);
  return (block *) (t - t % sizeof(void *));
}

void set_prev(block *b, block *to_set) {
  if(has_next(b)) {
    unset_next(b);
    b->prev = to_set;
    set_next(b);
    return;
  } 
  b->prev = to_set;
}

block *get_header(void *addr) {
  // sizeof(void *)*2 - because we have 2 block variables above our data pointer
  // and mem_alloc returns block->data pointer
  return (block *)(addr - sizeof(void *)*2);
}
