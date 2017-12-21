#include "syscall.h"

#define SIZE 10

void* lock;
void* free_to_fill;
void* ready_to_consume;

int in = 0, out = 0;
char buffer[10];

void prod(void *arg) {
	while (1){
		sem_wait(free_to_fill);
		sem_wait(lock);
		
		buffer[in++] = GetChar();
		in = in % SIZE;
		
		sem_post(ready_to_consume);
		
		sem_post(lock);
		if (buffer[in - 1] == 'q')
			return;
		Yield();
	}
}
void consume(void *arg) {
	while (1){
		sem_wait(ready_to_consume);
		sem_wait(lock);
		
		PutString("Consuming: ");
		PutChar(buffer[out++]);
		PutChar('\n');
		out = out % SIZE;
		
		sem_post(free_to_fill);
		
		sem_post(lock);
		if (buffer[in - 1] == 'q')
			return;
		Yield();
	}
}

int main() {
	
	sem_init(&lock, 1);
	sem_init(&free_to_fill, SIZE);
	sem_init(&ready_to_consume, 0);

    PutString("In main\n");
	PutString("Creating producer #");
	PutInt(UserThreadCreate(prod, (void *) 0));
	PutString("\nCreating consumer #");
	PutInt(UserThreadCreate(consume, (void *) 1));
	PutString("\nCreating consumer #");
	PutInt(UserThreadCreate(consume, (void *) 2));
	
	UserThreadJoin(1);
	UserThreadJoin(2);
	UserThreadJoin(3);
	
	PutString("\nHalting...");
    Halt();
}
