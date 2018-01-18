#include "syscall.h"

#define SIZE 10

sema_t lock;
sema_t free_to_fill;
sema_t ready_to_consume;

int in = 0, out = 0;
char buffer[10];

void* prod(void *arg) {
	while (1){
		sem_wait(free_to_fill);
		sem_wait(lock);
		
		buffer[in++] = GetChar();
		in = in % SIZE;
		
		sem_post(ready_to_consume);
		
		sem_post(lock);
		if (buffer[in - 1] == 'q')
			break;
		Yield();
	}
	return NULL;
}
void* consume(void *arg) {
	while (1){
		sem_wait(ready_to_consume);
		sem_wait(lock);

		PutInt((int)arg);
		PutString(" - Consuming: ");
		if (buffer[out] == 'q'){		
			sem_post(ready_to_consume);			
			sem_post(lock);
			break;
		}
		PutChar(buffer[out++]);
		PutChar('\n');
		out = out % SIZE;
		
		sem_post(free_to_fill);
		
		sem_post(lock);
		Yield();
	}
	PutString("Consumer#");
	PutInt((int)arg);
	PutString(" is done\n");
	return (void*)arg;
}

int main() {
	
	sem_init(&lock, 1);
	sem_init(&free_to_fill, SIZE);
	sem_init(&ready_to_consume, 0);

	int n_consumer = 10;

    PutString("In main\n");
	PutString("Creating producer #");
	PutInt(UserThreadCreate(prod, (void *) 0));

	for (int i = 1; i <= n_consumer; i++){
		PutString("\nCreating consumer #");
		PutInt(UserThreadCreate(consume, (void *) i));
	}
	int result;
	for (int i = 1; i <= n_consumer; i++)
		UserThreadJoin(i + 1, &result);

	return result;
}
