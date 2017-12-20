#include "syscall.h"

void func(void *arg) {
    int i, n = (int) arg;

    for (i = 0; i < n; i++) {
        PutString("func ");
        PutInt(i);
        PutChar('\n');
    }

    UserThreadExit();
}

int main() {

    UserThreadCreate(func, (void *) 5);
    Halt();
}
