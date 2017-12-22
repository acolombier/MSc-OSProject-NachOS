#include "../mem_alloc.h"
#include <stdio.h>
#include <string.h>

#define PAYLOADSIZE_CHANGE 0
#define UP_BOUDARIES_OVERWRITE 1
#define SELF_ALLOCATED 2
#define CORRECT_BEHAVIOR 3

unsigned int test_case=CORRECT_BEHAVIOR;

int main(int argc, char *argv[]) {
	char *ptr1 = NULL, *ptr2 = NULL, *ptr3 = NULL;
	
	if (argc > 1)
		test_case = atoi(argv[1]);
		
	if (test_case < PAYLOADSIZE_CHANGE || test_case > CORRECT_BEHAVIOR || !strcmp(argv[1], "--help")){
		fprintf(stderr, "Usage: %s <testcase_id>\n\ttestcase_id: \t0 - Change the size of the payload\n\t\t\t1 - Override the next footer/header\n\t\t\t2 - Try to allocated the next block by overidding the next header\n\t\t\t3 - Correct behavior\n\n\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	switch (test_case){
		case PAYLOADSIZE_CHANGE:
			fprintf(stderr, "--------------\nTestCase: PAYLOADSIZE_CHANGE\n--------------\n\n");
			ptr1 = (char*)malloc(16);
			ptr2 = (char*)malloc(96);
			
			ptr1 -= 16;
			fprintf(stderr, "Current size: %d\n", (int)*ptr1);
			*ptr1 = 48;	
			fprintf(stderr, "New size: %d\n", (int)*ptr1);
			ptr3 = (char*)malloc(96);
			free(ptr1 + 16);
			free(ptr2);
			free(ptr3);
			break;
		case SELF_ALLOCATED:
			fprintf(stderr, "--------------\nTestCase: SELF_ALLOCATED\n--------------\n\n");
			ptr1 = (char*)malloc(16);
			ptr2 = (char*)malloc(96);
			
			ptr2 = ptr1 + 32;
			memset(ptr1, 0, 32);
			*((int*)ptr1) = 256;	
			*((char*)(((int*)ptr1) + 1)) = 1;	
			
			ptr3 = (char*)malloc(96);
			free(ptr1);
			free(ptr2);
			free(ptr3);
			break;
		case UP_BOUDARIES_OVERWRITE:
			fprintf(stderr, "--------------\nTestCase: UP_BOUDARIES_OVERWRITE\n--------------\n\n");
			ptr1 = (char*)malloc(16);
			ptr2 = (char*)malloc(96);
			
			ptr1 += 16;
			fprintf(stderr, "Current size: %d\n", (int)*ptr1);
			memset(ptr1, 0, 32);
			
			ptr3 = ptr3;
			free(ptr1 - 16);
			free(ptr2);
			break;
		default:
			fprintf(stderr, "--------------\nTestCase: CORRECT_BEHAVIOR\n--------------\n\n");
			ptr1 = (char*)malloc(16);
			ptr2 = (char*)malloc(96);
			ptr3 = (char*)malloc(112);
			
			memset(ptr1, 0, 16);
			memset(ptr2, 1, 96);
			memcpy(ptr3, ptr2, 96);	
			memcpy(ptr3 + 96, ptr1, 16);
				
			free(ptr1);
			free(ptr2);
			free(ptr3);
			break;
	}
	return EXIT_SUCCESS;
}
