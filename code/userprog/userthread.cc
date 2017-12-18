#include "system.h"
#include "userthreads.h"

typedef struct bundle {
    int function;
    int arg;
} bundle_t;

//static void StartUserThread(int f);

int do_UserThreadCreate(int f, int arg) {
    Thread *t = new Thread("");

    bundle_t bundle = {f, arg};

    // the new user thread needs to share its space with the old one
    t->space = currentThread->space;

    // because our new thread will be a user thread
    // its address space should not be NULL
    if (t->space == NULL) {
        return -1;
    }

    t->Fork(StartUserThread, bundle);

    return 0;
}
