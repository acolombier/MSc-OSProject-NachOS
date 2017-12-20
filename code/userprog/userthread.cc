#include "system.h"
#include "userthread.h"

/*! A struct to pass the user function and args to StartUserThread */
typedef struct bundle {
    int function;  /*!< A user pointer to the function that will be execute by the user thread */
    int arg;  /*!< A user pointer to the arguments of that function */
} bundle_t;

/*! Function that starts the user thread */
/*!
 * \param bundle_p Pointer to a bundle struct that contain the user function and args to be executed.
 */
/*! \todo technical documentation on technical choices */
static void StartUserThread(int bundle_p) {

    bundle_t *bundle = (bundle_t *) bundle_p;
    DEBUG('t', "Inside StartUserThread\n");

    fprintf(stderr, "userThread to run %p(%p)\n", (void*)bundle->function, (void*)bundle->arg);

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

    t->Fork(StartUserThread, (int) bundle);

    return 0;
}

void do_UserThreadExit() {
    // we destroy the kernel thread because it was created
    // only to run the user thread
    // TODO: disable interrupts (probably)
    currentThread->Finish();
}

int do_UserThreadJoin(int tid) {
    // TODO: the function!!!
    return -1;
}
