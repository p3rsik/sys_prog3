#include "lib.h"
#include "stdio.h"

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
  } 
  b->prev = to_set;
}

block *get_header(void *addr) {
  // sizeof(void *)*2 - because we have 2 block variables above our data pointer
  // and mem_alloc returns block->data pointer
  return (block *)(addr - sizeof(void *)*2);
}

size_t alloc_size(size_t size) {
  // we need to allocate additional memory for block header
  // and substract one pointer from there, since our data segments starts in header
  return sizeof(block) + size - sizeof(void *);
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
  
  for(;(cur != NULL) && (is_used(cur) != 0) && (get_size(cur) < size); cur = get_next(cur));

  return cur;
}

block *split_block(block *to_split, size_t size) {
  // basically *to_split* becomes our block, and we "allocate" new block after this one
  block *new_header = to_split->data + size;
  // we can omit calls to get_size here,
  // since we knew that splitting only occurs on free blocks
  new_header->size = to_split->size - size - sizeof(block);
  if(has_next(to_split)) {
    set_next(new_header);
  } 
  set_prev(new_header, to_split);
  set_next(to_split);
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
    set_next(top);
    set_prev(b, top);
  }
  top = b;

  return b->data;
}

void mem_copy(void *addr1, void *addr2, size_t size) {
  char *a1 = (char *) addr1, *a2 = (char *) addr2;
  for(size_t i = 0; i < size; i++)
    *(a1 + i) = *(a2 + i);
}

void *mem_realloc(void *addr, size_t size) {
  if(!addr) {
    return mem_alloc(size);
  }
  // save block
  block *old = get_header(addr);

  mem_free(addr);
  block *new = mem_alloc(size);
  if(!new) {
    mem_alloc(get_size(old));
  }
}

block *merge_blocks(block *b1, block *b2) {
  // new size equals to sum of data segments + sizeof block header 
  // - sizeof pointer to data segment of the second block
  b1->size = b2->size + sizeof(block) - sizeof(void *);
  // We don't need to set_next b1, since it had next in the past,
  // but we need to unset it, if b2 doesn't have next
  if(has_next(b2)) {
    set_next(b1);
    block *b = get_next(b2);
    set_prev(b, b1);
  } else { unset_next(b1); }

  return b1;
}

void mem_free(void *addr) {
  block *b = get_header(addr);
  block *to_merge = get_next(b);
  
  // Setting *used* bit flag to 0
  b->size = b->size - 1;
  // now we can use actual block.size here
  // because we are sure, that all blocks are free
  if (to_merge && is_used(to_merge) == 0) {
    b = merge_blocks(b, to_merge);
  }
  to_merge = get_prev(b);
  if(to_merge && is_used(to_merge) == 0) {
    merge_blocks(to_merge, b);
  }
}

void pprint(block *b) {
  printf("block: %p {\n\tsize: %d,\n\tused: %d,\n\tnext: %p,\n\tprev: %p,\n\tdata: %p\n}\n",
         b, get_size(b), is_used(b), get_next(b), get_prev(b), b->data);
}

void mem_dump() {
  block *b = heap_start;
  for(;get_next(b);b = get_next(b)) {
    pprint(b);
    printf("\n");
  }
  pprint(b);
  printf("\n");
}
