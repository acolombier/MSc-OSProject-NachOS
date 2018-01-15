#include "system.h"
#include "addrspace.h"
#include "userprocess.h"
#include "stringtransfer.h"

/*! A struct to pass the user function and args to StartUserProcess */
typedef struct bundle {
    int argc;  /*!< A user pointer to the arguments of that function */
    char** argv;  /*!< A user pointer to the arguments of that function */
    int exit_handler;  /*!< A user pointer to the library call of used to exit a process. Used to define the default return address */
} bundle_t;

/*! Function that starts the user process */
/*!
 * \param bundle_p Pointer to a bundle struct that contain the user function and args to be executed.
 */
/*! \todo technical documentation on technical choices */
static void StartUserProcess(int bundle_p) {

    bundle_t *bundle = (bundle_t *) bundle_p;
	
	DEBUG('t', "FORK_INIT: The working thread is %s#%d\n", currentThread->getName(), currentThread->tid());
    
	currentThread->space->InitRegisters ();	// set the initial register values

    if (bundle){
        machine->WriteRegister (4, bundle->argc); // argc init
        
        if (bundle->argv){
            int stack = machine->ReadRegister(StackReg);           
            int *arg_ptr = new int[bundle->argc];
            
            DEBUG('C', "Fork: Stack point to %d\n", stack);
            
            for (int c = 0; c < bundle->argc; c++){
                unsigned int arg_size = copyStringToMachine(bundle->argv[c], stack - strlen(bundle->argv[c]) - 1, MAX_STRING_SIZE);
                arg_ptr[c] = stack - strlen(bundle->argv[c]) - 1;
                delete [] bundle->argv[c];
                stack -= arg_size + 1;
            }
            stack = divRoundDown(stack, 4) * 4 - (bundle->argc) * 4 ;
            machine->WriteRegister (5, bundle->argc ? stack : 0);
            for (int c = 0; c < bundle->argc; c++){
                machine->WriteMem(stack + (4 * c), 4, arg_ptr[c]);
                DEBUG('C', "Fork: %d: %d => %d\n", c, stack + (4 * c), arg_ptr[c]);
            }
                
            free(bundle->argv);
            delete [] arg_ptr;
            
            machine->WriteRegister(StackReg, stack - 8);
        }
        
		machine->WriteRegister(RetAddrReg, bundle->exit_handler);

		delete bundle;
	}
	
    // call this function to run the user code
    machine->Run();
}

SpaceId do_UserProcessCreate(OpenFile* executable, int argc, char** argv, int exit_handler) {
	currentThread->SaveUserState();
	currentThread->space->SaveState();
	
	AddrSpace* space;    
    
    try {
        space = new AddrSpace (executable);
    } catch (PermissionException*){
        DEBUG('C', "Fork: File has not execution permission\n");
        return 0;
    } catch (ExecutableException*){
        DEBUG('C', "Fork: File is not a MIPS executable file\n");
        return 0;
    }
	space->ppid(currentThread->space->pid());
	
	Thread* t = new Thread("forkExec");
	space->appendThread(t);

	fileSystem->Close(executable);		// close file
    
    if (t->space == space){
        bundle_t *bundle = new bundle_t;
        bundle->argc = argc;
        bundle->argv = argv;
        bundle->exit_handler = exit_handler;

        t->Fork(StartUserProcess, (int) bundle);
        
        //~ currentThread->RestoreUserState();
        //~ currentThread->space->RestoreState();
        
        return space->pid();
    } else {
        delete t;
        delete space;
        return 0;
    }
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
		if (thread_to_join && thread_to_join != currentThread->tid()){				
			DEBUG('t', "--Halt call: Joining with thread #%d...\n", thread_to_join);
			currentThread->join(thread_to_join, 0);
			DEBUG('t', "--Halt call: Thread #%d has joined\n", thread_to_join);
			e = currentThread->space->threadList()->getFirst(); // To be sure that we don't have a de-allocated 'e'
		}
	} while (currentThread->space->countThread() > 1 && (e = e->next));
    currentThread->space->removeThread(currentThread, 255);
	interrupt->Halt();
}

