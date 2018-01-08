#include "system.h"
#include "addrspace.h"
#include "userthread.h"

/*! A struct to pass the user function and args to StartUserThread */
typedef struct bundle {
    int function;  /*!< A user pointer to the function that will be execute by the user thread */
    int arg;  /*!< A user pointer to the arguments of that function */
    int exit_handler;  /*!< A user pointer to the library call of used to exit a thread. Used to define the default return address */
} bundle_t;

/*! Function that starts the user thread */
/*!
 * \param bundle_p Pointer to a bundle struct that contain the user function and args to be executed.
 */
/*! \todo technical documentation on technical choices */
static void StartUserThread(int bundle_p) {

    bundle_t *bundle = (bundle_t *) bundle_p;

    currentThread->space->InitRegisters();  // init registers, including stack pointer

    machine->WriteRegister(4, bundle->arg);
    machine->WriteRegister(PCReg, bundle->function);
    machine->WriteRegister(NextPCReg, bundle->function + 4);
    machine->WriteRegister(RetAddrReg, bundle->exit_handler);

    delete bundle;

    // call this function to run the user code
    machine->Run();
}

tid_t do_UserThreadCreate(int f, int arg, int exit_handler) {
    Thread *t = new Thread("new User Thread");
    
    // the new user thread needs to share its space with the old one
    currentThread->space->appendThread(t);

    // because our new thread will be a user thread
    // its address space should not be NULL
    if (t->space == NULL) {
		delete t;
        return NULL_TID;
    }

    bundle_t *bundle = new bundle_t;
    bundle->function = f;
    bundle->arg = arg;
    bundle->exit_handler = exit_handler;

    t->Fork(StartUserThread, (int) bundle);

    return t->tid();
}

void do_UserThreadExit(/*int code*/) {
    //~ currentThread->setResult(code);
	currentThread->Finish();
}

int do_UserThreadJoin(tid_t tid) {
	return currentThread->join(tid);
}
