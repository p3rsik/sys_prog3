#include "lib.h"

size_t get_size(block *b) {
  return b->size - (b->size % sizeof(void *));
}

unsigned char is_used(block *b) {
  return b->size % sizeof(void *);
}

block *get_header(void *addr) {
  // sizeof(void *)*3 - because we have 3 block variables above our data pointer
  // and mem_alloc returns block->data pointer
  return (block *)(addr - sizeof(void *)*3);
}

size_t alloc_size(size_t size) {
  // we need to allocate additional memory for block header
  return sizeof(block) + size;
}

block *request_from_os(size_t size) {
  // get current heap break
  block *b = (block *) sbrk(0);
  
  if (sbrk(alloc_size(size)) == (void *)-1) {
    return NULL;
  }

  return b;
}

size_t align(size_t n) {
  return (n + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
}

block *find_block(size_t size) {
  if(!heap_start) {
    return NULL;
  }
  block *cur = heap_start;
  
  for(;(cur != NULL) && (is_used(cur) != 0) && (get_size(cur) < size); cur = cur->next);

  return cur;
}

block *split_block(block *to_split, size_t size) {
  // basically *to_split* becomes our block, and we "allocate" new block after this one
  block *new_header = to_split->data + size;
  // we can omit calls to get_size here,
  // since we knew that splitting only occurs on free blocks
  new_header->size = to_split->size - size - sizeof(block);
  new_header->prev = to_split;
  new_header->next = to_split->next;
  to_split->next = new_header;
  new_header->data = to_split->data + size + sizeof(block) - sizeof(void *);
  to_split->size = size;

  return to_split;
}

void *mem_alloc(size_t size) {
  size = align(size);

  block *b;

  // Try to find block to reuse
  if(b = find_block(size)) {
    // It's impossible to split block if it's data segment 
    // can't contain new block of minimum aligned size + header
    if(b->size >= (size + sizeof(block)) + sizeof(void *)) {
      b = split_block(b, size);
      // Encoding *used* bit flag here
      b->size += 1;
    }
    return b->data;
  }
  
  // if not found, request new memory from OS
  b = request_from_os(size);
  if (!b) {
    return NULL;
  }
  // Encoding *used* bit flag here
  b->size = size + 1;
  b->data = (void *)(((void *) b) + sizeof(block) - sizeof(void *));

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
  block *b = get_header(addr);
  block *to_merge = b->next;
  
  // Setting *used* bit flag to 0
  b->size = b->size - 1;
  // now we can use actual block.size here
  // because we are sure, that all blocks are free
  if (to_merge && is_used(to_merge) == 0) {
    b->size = b->size + to_merge->size + sizeof(block);
    b->next = to_merge->next;
  }
  if(b->prev && is_used(b->prev) == 0) {
    to_merge = b->prev;
    to_merge->size = to_merge->size + b->size + sizeof(block);
    to_merge->next = b->next;
  }
}
