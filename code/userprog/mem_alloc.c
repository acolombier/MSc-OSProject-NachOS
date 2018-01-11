#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "mem_alloc_types.h"

/* memory */
int BLOCK_SIZE_WITH_PADDING = (sizeof(mem_block) % MEM_ALIGNMENT ? (sizeof(mem_block) / MEM_ALIGNMENT + 1) * MEM_ALIGNMENT : sizeof(mem_block));

char memory[MEMORY_SIZE];

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

void run_at_exit(void)
{    
    if (!IS_BLOCK_FREE(((mem_block*)memory)) || ((mem_block*)memory)->size != MEMORY_SIZE - 2 * BLOCK_SIZE_WITH_PADDING){
		fprintf(stderr, "Leak detected!\n");   
		memory_display_state();
		return;
	}
}



void memory_init(void){

    /* register the function that will be called when the programs exits*/
    atexit(run_at_exit);
    
    memset(memory, 0, sizeof(mem_block));
    
	//~ fprintf(stdout, "METASIZE : %lu\n", sizeof(mem_block));		
    
    mem_block *first_block = (mem_block*)memory;
    first_block->flag = FREE_BLOCK;
    first_block->size = MEMORY_SIZE - 2 * BLOCK_SIZE_WITH_PADDING;
    
    DUMP_HEADER_TO_FOOTER(first_block);

    /* .... */
}

char *memory_alloc(int size){
    char * mem = NULL;
    
	mem_block *temporary_block = NULL, *current_block = (mem_block *)memory;
    
    //Rounding up size in order to keep mem alignment
    int effective_size = size;			
    
	if (effective_size % MEM_ALIGNMENT)
		effective_size = (effective_size / MEM_ALIGNMENT + 1) * MEM_ALIGNMENT;

#if defined(BEST_FIT) || defined(WORSE_FIT)

	mem_block *elected_block = NULL;
	int lost_bytes = MEMORY_SIZE;

#endif   
    
	//~ fprintf(stdout, "ALLOCATING, %p, %p\n", memory + MEMORY_SIZE, (char*)current_block);
    while (memory + MEMORY_SIZE > (char*)current_block){	
	if (!check_block(current_block)){
		fprintf(stderr, "Memory corrupted: Block at %lu doesn't match with the cheksum (current: %#02x)\n", ULONG((char*)current_block - memory), current_block->flag);	
		exit(EXIT_FAILURE);			
	}
#if defined(FIRST_FIT)

/* code specific to first fit strategy can be inserted here */			
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
		print_alloc_info(mem, size);
	}
	else print_alloc_error(size);
	
    return mem; /* to be modified */
}

void memory_free(char *p){   
	if (!p)
		return;
		
    mem_block* meta_block_to_free = (mem_block*)(p - BLOCK_SIZE_WITH_PADDING), *meta_previous_block = PREV_FOOTER_BLOCK(meta_block_to_free), *meta_next_block = NEXT_HEADER_BLOCK(meta_block_to_free);
    
	if (!check_block(meta_block_to_free)){
		fprintf(stderr, "Memory corrupted: Block at %lu doesn't match with the cheksum (current: %#02x)\n", ULONG((char*)meta_block_to_free - memory), meta_block_to_free->flag);	
		exit(EXIT_FAILURE);			
	}		
	
	//~ fprintf(stdout, "FREEING %d blocks... \n", block_size);		
	
	print_free_info(p);
    
	//~ fprintf(stderr, "Previous: %c\n", meta_previous_block->flag);
    if ((char*)meta_previous_block >= memory && IS_BLOCK_FREE(meta_previous_block)){ // The previous block is free
		//~ fprintf(stderr, "... is just before the freeing area\n");	
		meta_previous_block = HEADER_FROM_FOOTER(meta_previous_block);
		meta_previous_block->size += meta_block_to_free->size + 2 * BLOCK_SIZE_WITH_PADDING;
		DUMP_HEADER_TO_FOOTER(meta_previous_block);
		meta_block_to_free = meta_previous_block;
	}
	
	if ((char*)meta_next_block + BLOCK_SIZE_WITH_PADDING < memory + MEMORY_SIZE && IS_BLOCK_FREE(meta_next_block)) // The next block is free
		meta_block_to_free->size += meta_next_block->size + 2 * BLOCK_SIZE_WITH_PADDING;
	
	meta_block_to_free->flag &= 0xFE;
	DUMP_HEADER_TO_FOOTER(meta_block_to_free);
    
}

void memory_display_state(void){

    mem_block* current_block = (mem_block*)memory;
    double c = (long unsigned int)memory;
    
	printf("Mem. usage: [");
    while (memory + MEMORY_SIZE > (char*)current_block){
		for (; (int)c < (long unsigned int)((char*)current_block + current_block->size + 2 * BLOCK_SIZE_WITH_PADDING); c+= (double)MEMORY_SIZE / MEMORY_DISPLAY_SIZE)
			printf(IS_BLOCK_FREE(current_block) ? "." : "U");
		current_block = (mem_block*)((char*)current_block + current_block->size + 2 * BLOCK_SIZE_WITH_PADDING);
	}
	printf("]\n");
    
}


void print_alloc_info(char *addr, int size){
    fprintf(stderr, "ALLOC at : %lu (%d byte(s))\n", 
            ULONG(addr - memory), size);
}


void print_free_info(char *addr){
    fprintf(stderr, "FREE  at : %lu \n", ULONG(addr - memory));
}

void print_alloc_error(int size) 
{
    fprintf(stderr, "ALLOC error : can't allocate %d bytes\n", size);
}

void print_info(void) {
  fprintf(stderr, "Memory : [%lu %lu] (%lu bytes)\n", (long unsigned int) memory, (long unsigned int) (memory+MEMORY_SIZE), (long unsigned int) (MEMORY_SIZE));
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
	if ((char*)sibbling_meta[RIGHT_SIBBLING] < memory + MEMORY_SIZE){
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
		
	if ((char*)sibbling_meta[RIGHT_SIBBLING] < memory + MEMORY_SIZE)
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
		&& ((char*)sibbling_meta[RIGHT_SIBBLING] >= memory + MEMORY_SIZE || computed_sibbling_sum == small_hash_from_right);
}


#ifdef MAIN
int main(int argc, char **argv){

  /* The main can be changed, it is *not* involved in tests */
  memory_init();
  print_info();
  int i ; 
  for( i = 0; i < 10; i++){
    char *b = memory_alloc(rand()%8);
    memory_free(b);
  }

  char * a = memory_alloc(15); 
  memory_free(a);


  a = memory_alloc(10);
  memory_free(a);

  fprintf(stderr,"%lu\n",(long unsigned int) (memory_alloc(9)));
  return EXIT_SUCCESS;
}
#endif 
