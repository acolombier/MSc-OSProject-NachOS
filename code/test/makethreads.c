#include "syscall.h"

void func(void *arg) {
    int i = 0;
    int n = (int) arg;

    while (i < n) {
        PutString("func ");
        PutInt(i);
    }
}

int main() {

    UserThreadCreate(func, (void *) 5);
    Halt();
}
