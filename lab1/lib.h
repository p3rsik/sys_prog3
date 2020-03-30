#include "stdint.h"
#include "unistd.h"

typedef struct block {
  // Block size
  // We're include bit_flag *used* in the last bit of size
  size_t size;
  // Pointer to the next and prev blocks
  struct block *next;
  struct block *prev;
  // Pointer to the user data
  void *data;
} block;

size_t get_size(block *);
unsigned char is_used(block*);

// Start of our memory in heap
static block *heap_start = NULL;
// Helper variable for linking of blocks
static block *top = NULL;

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
void *mem_realloc(void *, size_t);
// Frees memory
void mem_free(void *);
