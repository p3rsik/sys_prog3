#include "allocator.h"
#include "stdio.h"

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
  if(DEBUG > 1) { printf("[FIND_BLOCK]: Got request to find block of size %d\n", size); }
  if(!heap_start) {
    if(DEBUG > 1) { printf("[FIND_BLOCK]: No block exist yet, exiting...\n"); }
    return NULL;
  }
  block *cur = heap_start;
  
  while(cur) {
    if(DEBUG > 1) {
      printf("[FIND_BLOCK]: ");
      pprint(cur);
    }
    if(!is_used(cur) && (get_size(cur) >= size)) {
      return cur;
    }
    if(DEBUG > 1) { printf("[FIND_BLOCK]: Block %p is used or smaller than requested..\n", cur); }
    cur = get_next(cur);
  }

  return NULL;
}

block *split_block(block *to_split, size_t size) {
  if(DEBUG > 1) { printf("[SPLIT_BLOCK]: Got request to cut %d bytes from the beginning of block %p\n", size, to_split); }
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
  if(DEBUG > 1) { printf("[SPLIT_BLOCK]: Split block into %p with size %d and %p with size %d\n", 
                         to_split, get_size(to_split), new_header, get_size(new_header)); }

  return to_split;
}

void *mem_alloc(size_t size) {
  if(DEBUG) { printf("[MEM_ALLOC]: Got request to allocate %d bytes\n", size); }
  size = align(size);
  if(DEBUG) { printf("[MEM_ALLOC]: Actually allocating %d bytes\n", size); }

  block *b;

  // Try to find block to reuse
  if(b = find_block(size)) {
    if(DEBUG) { printf("[MEM_ALLOC]: Found block %p with size %d\n", b, get_size(b)); }
    // It's impossible to split block if it's data segment 
    // can't contain new block of minimum aligned size + header
    if(b->size >= (size + sizeof(block)) + sizeof(void *)) {
      if(DEBUG) { printf("[MEM_ALLOC]: Can split this block...\n"); }
      b = split_block(b, size);
      // Encoding *used* bit flag here
      b->size += 1;
    }
    return b->data;
  }
  
  // if not found, request new memory from OS
  if(DEBUG) { printf("[MEM_ALLOC]: Found no block that fits the requirements, requesting memory from OS...\n"); }
  b = request_from_os(size);
  if (!b) {
    if(DEBUG) { printf("[MEM_ALLOC]: Request unseccessful...\n"); }
    return NULL;
  }
  // Encoding *used* bit flag here
  b->size = size + 1;
  b->data = (void *) (((void *) b) + sizeof(block) - sizeof(void *));

  // Init heap
  if(heap_start == NULL) {
    if(DEBUG) { printf("[MEM_ALLOC]: No block exists yet, initing heap_start pointer...\n"); }
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
  if(DEBUG > 1) { printf("[MEM_COPY]: Copying memory of size %d from %p to %p\n", size, addr2, addr1); }
  char *a1 = ((char *) addr1) + sizeof(void *), *a2 = (char *) addr2;
  for(size_t i = 0; i < size; i++) {
    if(DEBUG > 1) { printf("[MEM_COPY]: Copying %d from %p to %p\n", *(a2 + i), (a2 + i), (a1 + i)); }
    *(a1 + i) = *(a2 + i);
  }
}

void *mem_realloc(void *addr, size_t size) {
  if(DEBUG) { printf("[MEM_REALLOC]: Gor request for reallocation of addr %p with new size %d\n", addr, size); }
  if(!addr) {
  if(DEBUG) { printf("[MEM_REALLOC]: Since addr = NULL allocating new block...\n"); }
    return mem_alloc(size);
  }
  // save block
  block *old = get_header(addr);
  
  if(get_size(old) >= size) {
    if(DEBUG) { printf("[MEM_REALLOC]: Since new size <= than old size returning old block...\n"); }
    return old;
  }
  
  block *new = get_header(mem_alloc(size));
  if(DEBUG) { printf("[MEM_REALLOC]: Allocated new block %p\n", new); }
  mem_copy(new->data, old->data, get_size(old));
  mem_free(addr);
  if(DEBUG) { printf("[MEM_REALLOC]: Copied old data to the new block and freed old block...\n"); }
  return new;
}

block *merge_blocks(block *b1, block *b2) {
  if(DEBUG > 1) { printf("[MERGE_BLOCKS]: Merging blocks %p and %p\n", b1, b2); }
  // new size equals to sum of data segments + sizeof block header 
  // - sizeof pointer to data segment of the second block
  b1->size = get_size(b1) + get_size(b2) + sizeof(block) - sizeof(void *);
  // We don't need to set_next b1, since it had next in the past,
  // but we need to unset it, if b2 doesn't have next
  if(has_next(b2)) {
    block *b = get_next(b2);
    if(DEBUG > 1) { printf("[MERGE_BLOCKS]: Block %p has next block %p\n", b2, b); }
    set_prev(b, b1);
  } else { unset_next(b1); }
  if(DEBUG > 1) { printf("[MERGE_BLOCKS]: New block has addr %p and size %d\n", b1, get_size(b1)); }

  return b1;
}

void mem_free(void *addr) {
  block *b = get_header(addr);
  if(DEBUG) { printf("[MEM_FREE]: Got request to free %p block\n", b); }
  block *to_merge = get_next(b);
  
  // Setting *used* bit flag to 0
  b->size = b->size - 1;
  // now we can use actual block.size here
  // because we are sure, that all blocks are free
  if (to_merge && !is_used(to_merge)) {
    if(DEBUG) { printf("[MEM_FREE]: Can merge this block and the next one...\n"); }
    b = merge_blocks(b, to_merge);
  }
  to_merge = get_prev(b);
  if(to_merge && !is_used(to_merge)) {
    if(DEBUG) { printf("[MEM_FREE]: Can merge prev block and this one...\n"); }
    merge_blocks(to_merge, b);
  }
}

void pprint(block *b) {
  printf("block: %p {\n\tsize: %d,\n\tused: %d,\n\thas_next: %d,\n\tnext: %p,\n\tprev: %p,\n\tdata: %p\n}\n",
         b, get_size(b), is_used(b), has_next(b), get_next(b), get_prev(b), b->data);
}

void mem_dump() {
  block *b = heap_start;
  while(b) {
    pprint(b);
    b = get_next(b);
  }
}
