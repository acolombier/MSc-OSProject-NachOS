#ifndef   	_MEM_ALLOC_TYPES_H_
#define   	_MEM_ALLOC_TYPES_H_

#define MEM_ALIGNMENT 4

#define MEMORY_DISPLAY_SIZE 50

#define FREE_BLOCK 0x0
#define USED_BLOCK 0x1

#define LEFT_SIBBLING 0
#define RIGHT_SIBBLING 1

/* Structure declaration for a free block */
//~ typedef struct memory_block_free{
    //~ int block_size;
    //~ struct memory_block_free *next;
    //~ /* ...*/
//~ } mem_bfree_t;

/* Structure declaration for an allocated block */
//~ typedef mem_bfree_t mem_balloc_t;

/* Specific metadata for allocated blocks */
//~ typedef struct memory_block_allocated{
     //~ int size;
    //~ /* ...*/
//~ } mem_balloc_t;

typedef struct memory_block{
     int size;
     unsigned char flag;
    /* ...*/
} mem_block;


#endif
