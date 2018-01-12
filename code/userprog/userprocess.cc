#include "system.h"
#include "addrspace.h"
#include "userprocess.h"

/*! A struct to pass the user function and args to StartUserProcess */
typedef struct bundle {
    int arg;  /*!< A user pointer to the arguments of that function */
    int exit_handler;  /*!< A user pointer to the library call of used to exit a process. Used to define the default return address */
} bundle_t;

/*! Function that starts the user process */
/*!
 * \param bundle_p Pointer to a bundle struct that contain the user function and args to be executed.
 */
/*! \todo technical documentation on technical choices */
static void StartUserProcess(int bundle_p) {

    bundle_t *bundle = (bundle_t *) bundle_p;
	
	currentThread->space->InitRegisters ();	// set the initial register values

	DEBUG('t', "FORK_INIT: Thread workin is %s#%d\n", currentThread->getName(), currentThread->tid());

    if (bundle){
		//~ machine->WriteRegister(4, bundle->arg);
		machine->WriteRegister(PCReg, 0);
		machine->WriteRegister(NextPCReg, 4);
		machine->WriteRegister(RetAddrReg, bundle->exit_handler);

		delete bundle;
	}
	
    // call this function to run the user code
    machine->Run();
}

SpaceId do_UserProcessCreate(OpenFile* executable/*, int argc, char** argv*/, int exit_handler) {
	currentThread->SaveUserState();
	currentThread->space->SaveState();
	
	AddrSpace* space = new AddrSpace (executable);
	space->ppid(currentThread->space->pid());
	
	Thread* t = new Thread("forkExec");
	space->appendThread(t);

	fileSystem->Close(executable);		// close file

    //~ bundle_t *bundle = 0;
    bundle_t *bundle = new bundle_t;
    //~ bundle->function = f;
    //~ bundle->arg = arg;
    bundle->exit_handler = exit_handler;

    t->Fork(StartUserProcess, (int) bundle);
	
	currentThread->RestoreUserState();
	currentThread->space->RestoreState();
	
    return space->pid();
}

int do_UserProcessJoin(SpaceId pid, int result_code_ptr){	
	return currentThread->space->join(pid, result_code_ptr);
}

void do_UserProcessExit(int code){
	DEBUG('a', "Going to delete a process, %d currently remaining.\n", AddrSpace::ADDR_SPACE_COUNT());
	if (AddrSpace::ADDR_SPACE_COUNT() > 1)
		currentThread->Finish(code);
	else {
		DEBUG('a', "Last process has exited, halting machine.\n");
		do_UserHalt();
	}
}

void do_UserHalt(){
	if (AddrSpace::ADDR_SPACE_COUNT() > 1){
		DEBUG('a', "Shutdown cancelled, other processus are still running.\n");
		currentThread->Finish();
		return;
	}
	
	ListElement*e = currentThread->space->threadList()->getFirst();
	do {
		tid_t thread_to_join = ((Thread*)e->item)->tid();	
		if (thread_to_join != currentThread->tid()){				
			DEBUG('t', "--Halt call: Joining with thread #%d...\n", thread_to_join);
			currentThread->join(thread_to_join, 0);
			DEBUG('t', "--Halt call: Thread #%d has joined\n", thread_to_join);
			e = currentThread->space->threadList()->getFirst(); // To be sure that we don't have a de-allocated 'e'
		}
	} while (currentThread->space->countThread() > 1 && (e = e->next));
	interrupt->Halt();
}

