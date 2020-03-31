#include "stdint.h"
#include "unistd.h"

typedef struct block {
  // Block size
  // We're include bit_flag *used* in the last bit of size
  size_t size;
  // Pointer to the prev block and *has_next* bit flag
  struct block *prev;
  // Pointer to the user data
  void *data;
} block;

// Get the actual size
size_t get_size(block *);
// Extract bit flag from size
unsigned char is_used(block *);
// Check if *next* bit flag is set
unsigned char has_next(block *);
// Get next block by adding size offset, 
// returns NULL if there is no next and pointer to the next if there is one
block *get_next(block *);
// Set and unset *next* bit flag
void set_next(block *);
void unset_next(block *);
// Get and set prev block
block *get_prev(block *);
void set_prev(block *, block *);

// Start of our memory in heap
static block *heap_start = NULL;
// Helper variable for linking of blocks
static block *top = NULL;

// Debug level
// 1 - for main functions(mem_alloc, mem_free, mem_realloc)
// 2 - for helper functions of main functions(merge_block, find_block...)
static int DEBUG = 2;

// Helper function to get block header from allocated memory
block *get_header(void *);

// Finds block to reuse that has size >= than requested
block *find_block(size_t);

// Splits block returning new block allocated at the start of the old one
block *split(block *, size_t);

// We need to ensure, that we can allocate our header structure on top of the block,
size_t alloc_size(size_t);

// Request memory from OS to map it
block *request_from_os(size_t);

// Aligns by pointer size
size_t align(size_t);

// Get a pointer to AT LEAST this much bytes
void *mem_alloc(size_t);

// Copies [size] bytes of memory from the second pointer into the first one
void mem_copy(void *, void *, size_t);
void *mem_realloc(void *, size_t);

// Merges second(b2) block into first one
block *merge_blocks(block *b1, block *b2);
// Frees memory
void mem_free(void *);

void pprint(block *);
void mem_dump();
