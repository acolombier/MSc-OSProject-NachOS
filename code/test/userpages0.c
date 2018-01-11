#include "syscall.h"

#define THIS "a"
#define THAT "b"

const int N = 5; // Choose it large enough!

void puts(char *s){
	char *p; for (p = s; *p != '\0'; p++) PutChar(*p);
	}

void* f(void *s){
	for (int i = 0; i < N; i++) puts((char *)s);
	return (void*)1001;
}

int main(){
	UserThreadCreate(f, (void *) THIS);
	f((void*) THAT);
	return 1000;
}
