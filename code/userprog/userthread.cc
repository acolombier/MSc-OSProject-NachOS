#include "system.h"
#include "userthread.h"
#define PAGESPERTHREAD 10

typedef struct bundle {
    int function;
    int arg;
} bundle_t;

static void StartUserThread (int f);

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

    t->Fork(StartUserThread, (int) &bundle);

    return 0;
}

void do_UserThreadExit() {
    // we destroy the kernel thread because it was created
    // only to run the user thread
    currentThread->Finish();
}

static void StartUserThread (int f){

    bundle_t *bundle = (bundle_t *) f;
    DEBUG('t', "Inside start user thread\n");

    fprintf(stderr, "SC %d %d\n", bundle->function, bundle->arg);

    currentThread->space->RestoreState();  // load addspace into the machine
    currentThread->space->InitRegisters();  // init registers, including stack pointer

    machine->WriteRegister(4, bundle->arg);
    machine->WriteRegister(PCReg, bundle->function);
    machine->WriteRegister(NextPCReg, bundle->function + 4);

    int pos = machine->ReadRegister(StackReg);
    // TODO: check if we are too close to the heap to have another stack
    if (pos < 0) {
        fprintf(stderr, "Insufficient Space for New Thread Creation \n");
        do_UserThreadExit();
    }
    currentThread->bitmapID= pos;

  	//fprintf(stderr, "%d\n", temp);
    // TODO initialize the new stack pointer for the usr program
  	machine->WriteRegister (StackReg,  PAGESPERTHREAD *(pos+1) * PageSize);
  	//machine->WriteRegister (StackReg, currentThread->space->numPages*PageSize -16 - 3 * tid * PageSize);

  	// call this function to run the system
  	machine->Run();
  }
}
