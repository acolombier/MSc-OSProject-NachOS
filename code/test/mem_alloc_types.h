#ifndef   	_MEM_ALLOC_TYPES_H_
#define   	_MEM_ALLOC_TYPES_H_

#define MEM_ALIGNMENT 4

#define MEMORY_DISPLAY_SIZE 50

#define FREE_BLOCK 0x0
#define USED_BLOCK 0x1

#define LEFT_SIBBLING 0
#define RIGHT_SIBBLING 1

typedef struct memory_block{
     int size;
     unsigned char flag;
} mem_block;


#endif
