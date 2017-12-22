#include "syscall.h"
//#include "synchconsole.h"
//#include "system.h"
//#include "bitmap.h"
//#include "synch.h"

void func(void *arg) {
    int i, n = (int) arg;

    PutString("\nIn func\n");
    for (i = 0; i < n; i++) {
        PutString("func ");
        PutInt(i);
    }

    UserThreadExit();
}

int main() {
	
	// TODO make more than 4 threads run!

    PutString("In main\n");
	PutString("Creating thread #");
	PutInt(UserThreadCreate(func, (void *) 5));
	PutString("\nCreating thread #");
	PutInt(UserThreadCreate(func, (void *) 10));
	PutString("\nCreating thread #");
	PutInt(UserThreadCreate(func, (void *) 15));
	
	PutString("\nCreating thread #");
	PutInt(UserThreadCreate(func, (void *) 5));
	
	PutString("\nHalting...");
    Halt();
}
