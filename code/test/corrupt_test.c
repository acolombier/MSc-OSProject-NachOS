#include "syscall.h"
#include "userlib.h"
#include "malloc.h"

#define PAYLOADSIZE_CHANGE 0
#define UP_BOUDARIES_OVERWRITE 1
#define SELF_ALLOCATED 2
#define CORRECT_BEHAVIOR 3


int main(int argc, char **argv) {
	char *ptr1 = NULL, *ptr2 = NULL, *ptr3 = NULL;

    unsigned int test_case=-1;
	
	if (argc > 1)
		test_case = atoi(argv[1]);
		
	if (test_case < PAYLOADSIZE_CHANGE || test_case > CORRECT_BEHAVIOR || !strcmp(argv[1], "--help")){
		PutString("Usage: ");PutString(argv[0]);PutString(" <testcase_id>\n\ttestcase_id: \t0 - Change the size of the payload\n\t\t\t1 - Override the next footer/header\n\t\t\t2 - Try to allocated the next block by overidding the next header\n\t\t\t3 - Correct behavior\n\n\n");
		return EXIT_FAILURE;
	}
	
	switch (test_case){
		case PAYLOADSIZE_CHANGE:
			PutString("--------------\nTestCase: PAYLOADSIZE_CHANGE\n--------------\n\n");
			ptr1 = (char*)malloc(16);
			ptr2 = (char*)malloc(96);
			
			ptr1 -= 8;
			PutString("Current size: ");PutInt((int)*ptr1);PutChar('\n');
			*ptr1 = 48;	
			PutString("New size: ");PutInt((int)*ptr1);PutChar('\n');
			ptr3 = (char*)malloc(96);
			free(ptr1 + 16);
			free(ptr2);
			free(ptr3);
			break;
		case SELF_ALLOCATED:
			PutString("--------------\nTestCase: SELF_ALLOCATED\n--------------\n\n");
			ptr1 = (char*)malloc(8);
			ptr2 = (char*)malloc(16);
			
			ptr2 = ptr1 + 8 + 4;
			memset(ptr1, 0, 16);
			*((int*)ptr1) = 40;	
			*((char*)(((int*)ptr1) + 1)) = 1;	
			
			ptr3 = (char*)malloc(32);
			free(ptr1);
			free(ptr2);
			free(ptr3);
			break;
		case UP_BOUDARIES_OVERWRITE:
			PutString("--------------\nTestCase: UP_BOUDARIES_OVERWRITE\n--------------\n\n");
			ptr1 = (char*)malloc(16);
			ptr2 = (char*)malloc(96);
			
			ptr1 += 16;
			PutString("Current size: ");PutInt((int)*ptr1);PutChar('\n');
			memset(ptr1, 0, 16);
			
			ptr3 = ptr3;
			free(ptr1 - 16);
			free(ptr2);
			break;
		default:
			PutString("--------------\nTestCase: CORRECT_BEHAVIOR\n--------------\n\n");
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
