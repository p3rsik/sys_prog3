#include "unistd.h"
#include "stdint.h"

typedef struct block {
  // Block size
  // We're include bit_flag *used* in the last bit of size
  size_t size;
  // Pointer to the prev block and *has_next* bit flag
  struct block *prev;
  // Pointer to the user data
  void *data;
} block;

// Helper function to get block header from allocated memory
block *get_header(void *);

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
