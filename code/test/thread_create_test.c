#include "syscall.h"

// Testing the thread creation
// Reminder :

void* wait_function(void *arg){
    for(int i = 10000; i > 0; i --){
    }
    PutString("Stopping thread ");
    PutInt((int) arg);
    PutChar('\n');
    return NULL;
}

int main(void){
    PutString("This program is supposed to reach cases where threads cannot be created\n");
    PutString("Type 1 to create too many threads\n");
    PutString("Type 2 to fill the memory and then try to create a thread\n");
    char answer = GetChar();
    while(answer < '1' || answer > '2'){
	PutString("Please answer by 1 or 2\n");
	answer = GetChar();
    }
    switch(answer){
    case '1' :
	PutString("Creating more threads than maximum\n");
	int thread_id;
	for(int i = 0; i < 17; i++){
	    thread_id = UserThreadCreate(wait_function, (void*) i+1);
	    if(thread_id != -1){
		PutString("Created thread ");
		PutInt(thread_id);
		PutChar('\n');
	    }
	    else{
		PutString("Could not create thread : recieved ");
		PutInt(thread_id);
		PutChar('\n');
	    }
	}
	break;
    default :
	PutString("Should not happen !!!!\n");
	Exit(EXIT_FAILURE);
    }
    Exit(EXIT_SUCCESS);
}
