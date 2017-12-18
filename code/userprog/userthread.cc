#include "system.h"
#include "userthread.h"

typedef struct bundle {
    int function;
    int arg;
} bundle_t;

void StartUserThread(int f) {
}

int do_UserThreadCreate(int f, int arg) {
    Thread *t = new Thread("");

    // the new user thread needs to share its space with the old one
    t->space = currentThread->space;

    // because our new thread will be a user thread
    // its address space should not be NULL
    if (t->space == NULL) {
        return -1;
    }

    bundle_t bundle = {f, arg};
    t->Fork(StartUserThread, (int) &bundle);

    return 0;
}

void do_UserThreadExit() {
    // we destroy the kernel thread because it was created
    // only to run the user thread
    currentThread->Finish();
}
