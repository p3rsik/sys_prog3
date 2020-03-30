#include "stdint.h"
#include "unistd.h"

struct block {
  // Block size
  size_t size;
  // Whether this block is currently used
  uint8_t used;
  // Pointers to the next and previous blocks
  struct block *next;
  struct block *prev;
  // Pointer to the user data
  void *data;
};

// Start of our memory in heap
static struct block *heap_start = NULL;
// Helper variable for linking of blocks
static struct block *top = NULL;

// Helper function to get block header from allocated memory
struct block *get_header(void *);

// Finds block to reuse that has size >= than requested
struct block *find_block(size_t);

// Splits block returning new block allocated at the start of the old one
struct block *split(struct block *, size_t);

// We need to ensure, that we can allocate our header structure on top of the block,
size_t alloc_size(size_t);

// Request memory from OS to map it
struct block *request_from_os(size_t);

// Aligns by pointer size
size_t align(size_t);

// Get a pointer to AT LEAST this much bytes
void *mem_alloc(size_t);
void *mem_realloc(void *, size_t);
// Frees memory
void mem_free(void *);
