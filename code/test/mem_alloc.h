

#include "mem_alloc_types.h"

#ifndef 	_MEM_ALLOC_H_
#define 	_MEM_ALLOC_H_

#define BEST_FIT

#define IS_BLOCK_FREE(b) ((b->flag & 0x1) == FREE_BLOCK)
#define BLOCK_SIZE_WITH_PADDING (sizeof(mem_block) % MEM_ALIGNMENT ? (sizeof(mem_block) / MEM_ALIGNMENT + 1) * MEM_ALIGNMENT : sizeof(mem_block))

#define ULONG(x)((unsigned int)(x))
#define max(x,y) (x>y?x:y)

/* Propagate header and the footer */
#define NEXT_HEADER_BLOCK(b) ((mem_block*)((char*)b + b->size + 2 * BLOCK_SIZE_WITH_PADDING))
#define PREV_FOOTER_BLOCK(b) ((mem_block*)((char*)b - BLOCK_SIZE_WITH_PADDING))

#define FOOTER_FROM_HEADER(b) ((mem_block*)((char*)b + b->size + BLOCK_SIZE_WITH_PADDING))
#define HEADER_FROM_FOOTER(b) ((mem_block*)((char*)b - b->size - BLOCK_SIZE_WITH_PADDING))

#define DUMP_HEADER_TO_FOOTER(b) do { memcpy((char*)b + b->size + BLOCK_SIZE_WITH_PADDING, (char*)b, BLOCK_SIZE_WITH_PADDING); compute_checksum(b); } while(0)


/* Allocator functions, to be implemented in mem_alloc.c */
void memory_init();
char *memory_alloc(int size);
void memory_free(char *p);

/*  */

void memory_display_state(void);

mem_block* check_memory(void); // Return the first corrupted block, NULL otherwise
void compute_checksum(mem_block*);
char check_block(mem_block*);


#endif  	/* !_MEM_ALLOC_H_ */
