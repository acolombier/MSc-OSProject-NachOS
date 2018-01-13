#include "syscall.h"
#include "userlib.h"

#define THIS "a"
#define THAT "b"
#define NB_THREAD 12

const int N = 5; // Choose it large enough!

void puts(char *s){
	char *p; for (p = s; *p != '\0'; p++) PutChar(*p);
    PutChar('\n');
}

void* f(void *s){
	for (int i = 0; i < N; i++) puts((char *)s);
	return (void*)1001;
}

int main(int argc, char** argv){
    int nb_thread = argc ? atoi(argv[0]) : NB_THREAD;
    char threads[nb_thread];
    
    PutString("Running ");PutInt(nb_thread);PutString(" threads...\n");
    
    for (int i = 0; i < nb_thread; i++){
        PutString("Creating thread ");PutInt(i);PutString("...\n");
        threads[i] = UserThreadCreate(f, (void *) (i % 2 ? THIS : THAT));
        PutString("Created as ");PutInt(threads[i]);PutString("\n");
    }
	
    for (int i = 0; i < nb_thread; i++){
        int result_code;
        PutString("Joining thread ");PutInt(threads[i]);PutString("...\n");
        UserThreadJoin(threads[i], &result_code);
        PutString("Joined thread ");PutInt(threads[i]);PutString(" with exit code ");PutInt(result_code);PutString("\n");
    }
	return 1000;
}
