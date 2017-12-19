#include "system.h"
#include "userthread.h"

typedef struct bundle {
    int function;
    int arg;
} bundle_t;

static void StartUserThread(int f) {

    bundle_t *bundle = (bundle_t *) f;
    DEBUG('t', "Inside StartUserThread\n");

    fprintf(stderr, "userThread to run %d(%d)\n", bundle->function, bundle->arg);

    currentThread->space->RestoreState();  // load addspace into the machine
    currentThread->space->InitRegisters();  // init registers, including stack pointer

    machine->WriteRegister(4, bundle->arg);
    machine->WriteRegister(PCReg, bundle->function);
    machine->WriteRegister(NextPCReg, bundle->function + 4);

    //delete bundle;

    int sp = machine->ReadRegister(StackReg);
    int new_sp = ((sp / PageSize) * PageSize) - (2 * PageSize);
    // we check if we are too close to the heap to have another stack
    // for now we decided (hope) that code and heap should fit into two pages
    if (new_sp < 2 * PageSize) {
        fprintf(stderr, "Insufficient Space for New Thread Creation\n");
        do_UserThreadExit();
    }

    machine->WriteRegister(StackReg, new_sp);

    // call this function to run the user code
    machine->Run();
}

int do_UserThreadCreate(int f, int arg) {
    Thread *t = new Thread("new User Thread");

    bundle_t *bundle = new bundle_t;
    bundle->function = f;
    bundle->arg = arg;

    // the new user thread needs to share its space with the old one
    t->space = currentThread->space;

    // because our new thread will be a user thread
    // its address space should not be NULL
    if (t->space == NULL) {
        return -1;
    }

    fprintf(stderr, " v v v\n");
    t->Fork(StartUserThread, (int) &bundle);
    fprintf(stderr, " ^ ^ ^\n");

    return 0;
}

void do_UserThreadExit() {
    // we destroy the kernel thread because it was created
    // only to run the user thread
    currentThread->Finish();
}
