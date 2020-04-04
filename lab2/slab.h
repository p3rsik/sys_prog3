// DEFAULT - slab divided in blocks of fixed size
// START - indicates that this slab starts continious chain of slabs 
// to accomodate allocation request of size bigger that one slab
// CONTINIOUS - indicates that this slab is continuation of slab chain
// END - indicates that this slab is the end of slab chain
typedef enum slab_type {
  DEFAULT,
  START,
  CONTINIOUS,
  END
} slab_type;

// Slab size in bytes
#define SLAB_SIZE 4096

// Slab - memory of fixed size divided in blocks of the same fixed size
typedef struct slab {
  slab_type type;
  size_t block_size;
  // points to the first free block in slab
  void *free_block;
  // points to the physical memory that corresponds to this slab
  // first sizeof(void *) bytes of each free block would point to the next free block
  void *data;
} slab;

slab_type get_slab_type(slab *);
size_t get_block_size(slab *);
void *get_free_block(slab *);
void *get_data(slab *);
// This function can be called only on free blocks to get the next one in the chain
void *get_next_block(void *);

