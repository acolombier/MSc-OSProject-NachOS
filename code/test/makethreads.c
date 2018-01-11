#include "syscall.h"
//#include "synchconsole.h"
//#include "system.h"
//#include "bitmap.h"
//#include "synch.h"

void* func(void *arg) {
    int i, n = (int) arg;

    PutString("\nIn func\n");
    for (i = 0; i < n; i++) {
        PutString("func ");
        PutInt(i);
	PutChar('\n');
    }

    //~ UserThreadExit();
	return NULL;
}

int main() {

    PutString("In main\n");
    PutString("Creating thread #");
    PutInt(UserThreadCreate(func, (void *) 5));
    PutString("\nCreating thread #");
    PutInt(UserThreadCreate(func, (void *) 10));
    PutString("\nCreating thread #");
    PutInt(UserThreadCreate(func, (void *) 15));
    PutString("\nHalting...");
    Halt();
}
