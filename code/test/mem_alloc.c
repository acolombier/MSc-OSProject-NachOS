#include "syscall.h"
#include "mem_alloc.h"
#include "mem_alloc_types.h"

/* memory */
int BLOCK_SIZE_WITH_PADDING = (sizeof(mem_block) % MEM_ALIGNMENT ? (sizeof(mem_block) / MEM_ALIGNMENT + 1) * MEM_ALIGNMENT : sizeof(mem_block));

char *memory;
size_t memory_size;

/* Pointer to the first free block in the memory */
//~ mem_block *first_free;


#define ULONG(x)((long unsigned int)(x))
#define max(x,y) (x>y?x:y)

/* Copy the header to the footer */
#define NEXT_HEADER_BLOCK(b) ((mem_block*)((char*)b + b->size + 2 * BLOCK_SIZE_WITH_PADDING))
#define PREV_FOOTER_BLOCK(b) ((mem_block*)((char*)b - BLOCK_SIZE_WITH_PADDING))

#define FOOTER_FROM_HEADER(b) ((mem_block*)((char*)b + b->size + BLOCK_SIZE_WITH_PADDING))
#define HEADER_FROM_FOOTER(b) ((mem_block*)((char*)b - b->size - BLOCK_SIZE_WITH_PADDING))

#define DUMP_HEADER_TO_FOOTER(b) do { memcpy((char*)b + b->size + BLOCK_SIZE_WITH_PADDING, (char*)b, BLOCK_SIZE_WITH_PADDING); compute_checksum(b); } while(0)
//~ #define DUMP_FOOTER_TO_HEADER(b) memcpy((char*)b - b->size - BLOCK_SIZE_WITH_PADDING, (char*)b, BLOCK_SIZE_WITH_PADDING); compute_checksum(b)


void *memory_init(size_t size) {

	memory = Sbrk(size);
	if (memory == (char *) -1) {
		memory_size = 0;
		return (void *) -1;
	}
	memory_size = size;

	mem_block *first_block = (mem_block*)memory;
	first_block->flag = FREE_BLOCK;
	first_block->size = memory_size - 2 * BLOCK_SIZE_WITH_PADDING;

	DUMP_HEADER_TO_FOOTER(first_block);

	return (void *) memory;
}

char *memory_alloc(int size){
	char *mem = NULL;

	mem_block *temporary_block = NULL, *current_block = (mem_block *)memory;

    //Rounding up size in order to keep mem alignment
    int effective_size = size;

	if (effective_size % MEM_ALIGNMENT)
		effective_size = (effective_size / MEM_ALIGNMENT + 1) * MEM_ALIGNMENT;

#if defined(BEST_FIT) || defined(WORSE_FIT)

	mem_block *elected_block = NULL;
	int lost_bytes = memory_size;

#endif

	//~ fprintf(stdout, "ALLOCATING, %p, %p\n", memory + memory_size, (char*)current_block);
    while (memory + memory_size > (char*)current_block){
#if defined(FIRST_FIT)

/* code specific to first fit strategy can be inserted here */
		if (!check_block(current_block)){
			fprintf(stderr, "Memory corrupted: Block at %lu doesn't match with the cheksum (current: %#02x)\n", ULONG((char*)current_block - memory), current_block->flag);
			exit(EXIT_FAILURE);
		}
		if (IS_BLOCK_FREE(current_block) && current_block->size >= effective_size){
			if (current_block->size - effective_size >  2 * BLOCK_SIZE_WITH_PADDING){
			/* If there is enought room remaining for writing a free block struct, we do it */

				temporary_block = (mem_block*)((char*)current_block + effective_size + 2 * BLOCK_SIZE_WITH_PADDING);

				// Padding the new block on the same alignment
				if ((long unsigned int)temporary_block % MEM_ALIGNMENT){
					effective_size += MEM_ALIGNMENT - (long unsigned int)temporary_block % MEM_ALIGNMENT;
					temporary_block = (mem_block*)(((long unsigned int)temporary_block / MEM_ALIGNMENT + 1) * MEM_ALIGNMENT);
				}

				memset(temporary_block, 0, BLOCK_SIZE_WITH_PADDING);
				// We update the size of the block from where we took the memory
				temporary_block->size = current_block->size - effective_size - 2 * BLOCK_SIZE_WITH_PADDING;
				//~ DUMP_HEADER_TO_FOOTER(temporary_block);
			} else
				effective_size = current_block->size;


			current_block->size = effective_size;
			current_block->flag |= USED_BLOCK;
			//~ DUMP_HEADER_TO_FOOTER(current_block);
			mem = (char*)current_block + BLOCK_SIZE_WITH_PADDING;
			break;
		} else
			// We put our value to the next theoretical block
			current_block = (mem_block*)((char*)current_block + current_block->size + 2 * BLOCK_SIZE_WITH_PADDING);

#elif defined(BEST_FIT) || defined(WORST_FIT)
	if (IS_BLOCK_FREE(current_block) && current_block->size >= effective_size &&
#if defined(BEST_FIT)
	current_block->size - effective_size > lost_bytes){
#else
	current_block->size - effective_size < lost_bytes){
#endif
		lost_bytes = current_block->size - effective_size;
		elected_block = current_block;
	}
	current_block = (mem_block*)((char*)current_block + current_block->size + 2 * BLOCK_SIZE_WITH_PADDING);

#endif
	}

#if defined(BEST_FIT) || defined(WORST_FIT)
/* code specific to best fit strategy can be inserted here */
	if (elected_block){
		if (lost_bytes >  2 * BLOCK_SIZE_WITH_PADDING){
		/* If there is enought room remaining for writing a free block struct, we do it */

			temporary_block = (mem_block*)((char*)elected_block + effective_size + 2 * BLOCK_SIZE_WITH_PADDING);

			// Padding the new block on the same alignment
			if ((long unsigned int)temporary_block % MEM_ALIGNMENT){
				effective_size += MEM_ALIGNMENT - (long unsigned int)temporary_block % MEM_ALIGNMENT;
				temporary_block = (mem_block*)(((long unsigned int)temporary_block / MEM_ALIGNMENT + 1) * MEM_ALIGNMENT);
			}
			memset(temporary_block, 0, BLOCK_SIZE_WITH_PADDING);
			// We update the size of the block from where we took the memory
			temporary_block->size = lost_bytes - 2 * BLOCK_SIZE_WITH_PADDING;
			DUMP_HEADER_TO_FOOTER(temporary_block);
		} else
			effective_size = elected_block->size;


		((mem_block*)elected_block)->size = effective_size;
		elected_block->flag |= USED_BLOCK;
		//~ DUMP_HEADER_TO_FOOTER(elected_block);
		mem = (char*)elected_block + BLOCK_SIZE_WITH_PADDING;
	}
#endif

	if (mem){
		if (temporary_block)
			DUMP_HEADER_TO_FOOTER(temporary_block);
		DUMP_HEADER_TO_FOOTER(((mem_block*)(mem - BLOCK_SIZE_WITH_PADDING)));
	}

    return mem; /* to be modified */
}

void memory_free(char *p){
	if (!p)
		return;

    mem_block* meta_block_to_free = (mem_block*)(p - BLOCK_SIZE_WITH_PADDING), *meta_previous_block = PREV_FOOTER_BLOCK(meta_block_to_free), *meta_next_block = NEXT_HEADER_BLOCK(meta_block_to_free);

	if (!check_block(meta_block_to_free)){
		Exit(EXIT_FAILURE);
	}

	//~ fprintf(stderr, "Previous: %c\n", meta_previous_block->flag);
    if ((char*)meta_previous_block >= memory && IS_BLOCK_FREE(meta_previous_block)){ // The previous block is free
		//~ fprintf(stderr, "... is just before the freeing area\n");
		meta_previous_block = HEADER_FROM_FOOTER(meta_previous_block);
		meta_previous_block->size += meta_block_to_free->size + 2 * BLOCK_SIZE_WITH_PADDING;
		DUMP_HEADER_TO_FOOTER(meta_previous_block);
		meta_block_to_free = meta_previous_block;
	}

	if ((char*)meta_next_block + BLOCK_SIZE_WITH_PADDING < memory + memory_size && IS_BLOCK_FREE(meta_next_block)) // The next block is free
		meta_block_to_free->size += meta_next_block->size + 2 * BLOCK_SIZE_WITH_PADDING;

	meta_block_to_free->flag &= 0xFE;
	DUMP_HEADER_TO_FOOTER(meta_block_to_free);

}

 // Return the first corrupted block, NULL otherwise
mem_block* check_memory(){
	return NULL;
}

void compute_checksum(mem_block* b){
	mem_block* sibbling_meta[2] = {PREV_FOOTER_BLOCK(b), NEXT_HEADER_BLOCK(b)};

	//~ fprintf(stderr, "Block #%lu, Hash header: %#02x, Hash footer: %#02x\n", ULONG((char*)b - memory), b->flag, FOOTER_FROM_HEADER(b)->flag);

	// HEADER: 4 bits for own checksum, 3 for sibbling (left) checksum

	if ((char*)sibbling_meta[LEFT_SIBBLING] > memory){
		b->flag = sibbling_meta[LEFT_SIBBLING]->size % 7 << 1 | (b->flag & 1);
		if (!IS_BLOCK_FREE(sibbling_meta[LEFT_SIBBLING]))
			b->flag =  ((~b->flag) & 0xE) | (b->flag & 0x1);
		sibbling_meta[LEFT_SIBBLING]->flag &= 0xF1;
		sibbling_meta[LEFT_SIBBLING]->flag |= b->size % 7 << 1;
	}
	b->flag = b->size % 15 << 4 | (b->flag & 0xF);

	// FOOTER: 4 bits for own checksum, 3 for sibbling (right) checksum
	if ((char*)sibbling_meta[RIGHT_SIBBLING] < memory + memory_size){
		FOOTER_FROM_HEADER(b)->flag = sibbling_meta[RIGHT_SIBBLING]->size % 7 << 1 | (FOOTER_FROM_HEADER(b)->flag & 1);
		if (!IS_BLOCK_FREE(sibbling_meta[RIGHT_SIBBLING]))
			FOOTER_FROM_HEADER(b)->flag =  ((~FOOTER_FROM_HEADER(b)->flag) & 0xE) | (FOOTER_FROM_HEADER(b)->flag & 0x1);
		sibbling_meta[RIGHT_SIBBLING]->flag &= 0xF1;
		sibbling_meta[RIGHT_SIBBLING]->flag |= b->size % 7 << 1;
	}
	FOOTER_FROM_HEADER(b)->flag = FOOTER_FROM_HEADER(b)->size % 15 << 4 | (FOOTER_FROM_HEADER(b)->flag & 0xF);

	// Inverse sum if allocated
	if (!IS_BLOCK_FREE(b)){
		sibbling_meta[RIGHT_SIBBLING]->flag = ((~sibbling_meta[RIGHT_SIBBLING]->flag) & 0xE) | (sibbling_meta[RIGHT_SIBBLING]->flag & 0xF1);
		sibbling_meta[LEFT_SIBBLING]->flag = ((~sibbling_meta[LEFT_SIBBLING]->flag) & 0xE) | (sibbling_meta[LEFT_SIBBLING]->flag & 0xF1);

		b->flag = ((~b->flag) & 0xF0) | (b->flag & 0xF);
		FOOTER_FROM_HEADER(b)->flag = ((~FOOTER_FROM_HEADER(b)->flag) & 0xF0) | (FOOTER_FROM_HEADER(b)->flag & 0xF);
	}

	//~ fprintf(stderr, "Block #%lu, Hash header: %#02x, Hash footer: %#02x\n", ULONG((char*)b - memory), b->flag, FOOTER_FROM_HEADER(b)->flag);
	//~ memory_display_state();
}

char check_block(mem_block* b){
	mem_block* sibbling_meta[2] = {PREV_FOOTER_BLOCK(b), NEXT_HEADER_BLOCK(b)};
	unsigned char small_hash_from_left = 0,  small_hash_from_right = 0;
	unsigned char extended_hash;
	unsigned char computed_sibbling_sum = b->size % 7,  computed_extended_hash = b->size % 15;

	if ((char*)sibbling_meta[LEFT_SIBBLING] > memory)
		small_hash_from_left = sibbling_meta[LEFT_SIBBLING]->flag >> 1 & 0x7;

	if ((char*)sibbling_meta[RIGHT_SIBBLING] < memory + memory_size)
		small_hash_from_right = sibbling_meta[RIGHT_SIBBLING]->flag >> 1 & 0x7;

	extended_hash = b->flag >> 4;

	if (!IS_BLOCK_FREE(b)){
		computed_extended_hash = (~computed_extended_hash) & 0xF;
		computed_sibbling_sum = (~computed_sibbling_sum) & 0x7;
	}

	//~ fprintf(stderr, "Block info: %d, %#02x\n", b->size, b->flag);
	//~ fprintf(stderr, "Block self check: %d,%d; Block left check: %d,%d; Block right check: %d,%d;\n", computed_extended_hash,extended_hash, computed_sibbling_sum, small_hash_from_left, computed_sibbling_sum, small_hash_from_right);

	return (computed_extended_hash == extended_hash)
		&& ((char*)sibbling_meta[LEFT_SIBBLING] < memory || computed_sibbling_sum == small_hash_from_left)
		&& ((char*)sibbling_meta[RIGHT_SIBBLING] >= memory + memory_size || computed_sibbling_sum == small_hash_from_right);
}
