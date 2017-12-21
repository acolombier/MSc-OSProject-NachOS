#include "syscall.h"

void prod(void *arg) {
	PutString("Moi c'est Jackie\n");
}
void consume(void *arg) {	
	PutString("Moi c'est Michel\n");
}

int main() {
    PutString("In main\n");
	PutString("Creating producer #");
	PutInt(UserThreadCreate(prod, (void *) 0));
	PutString("\nCreating consumer #");
	PutInt(UserThreadCreate(consume, (void *) 1));
    Halt();
}
