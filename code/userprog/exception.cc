/*! \file exception.cc
	Entry point into the Nachos kernel from user programs.
	There are two kinds of things that can cause control to
	transfer back to here from user code:
	syscall -- The user code explicitly requests to call a procedure
	in the Nachos kernel.  Right now, the only function we support is
	"Halt".
	exceptions -- The user code does something that the CPU can't handle.
	For instance, accessing memory that doesn't exist, arithmetic errors,
	etc.
	Interrupts (which can also cause control to transfer from user
	code into the Nachos kernel) are handled elsewhere.
 */

// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "stringtransfer.h"
#include "userthread.h"
#include "addrspace.h"

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
static void
UpdatePC ()
{
	int pc = machine->ReadRegister (PCReg);
	machine->WriteRegister (PrevPCReg, pc);
	pc = machine->ReadRegister (NextPCReg);
	machine->WriteRegister (PCReg, pc);
	pc += 4;
	machine->WriteRegister (NextPCReg, pc);
}

//! ExceptionHandler
/*!
* Entry point into the Nachos kernel.  Called when a user program
* is executing, and either does a syscall, or generates an addressing
* or arithmetic exception.
*
* For system calls, the following is the calling convention:
*
* system call code -- r2
*             arg1 -- r4
*             arg2 -- r5
*             arg3 -- r6
*             arg4 -- r7
*
* The result of the system call, if any, must be put back into r2.
*
* \param which The kind of exception to handle. The list of possible exceptions
* is in machine.h under enum \ref ExceptionType.
*/

//----------------------------------------------------------------------
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
// which" is the kind of exception.  The list of possible exceptions
// are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler (ExceptionType which)
{
	int type = machine->ReadRegister(2);
	// argument registers used by the syscall functions
	int reg4, reg5, reg6, returnvalue; // , , reg7;


	/*! \todo Optimisation by reading only useful register for a given trap */
	reg4 = machine->ReadRegister(4);
	reg5 = machine->ReadRegister(5);
	reg6 = machine->ReadRegister(6);
	//reg7 = machine->ReadRegister(7);


	if (which == SyscallException) {
		switch (type) {

			case SC_Halt: {
				DEBUG('a', "Shutdown, initiated by user program.\n");
				// the prog shouldn't exit while the thread is still running
				// if it has exited before; then nothing happens here
				// otherwise the halting of the main is made clean
				ListElement*e = currentThread->space->threadList()->getFirst();
				do {
					tid_t thread_to_join = ((Thread*)e->item)->tid();	
					if (thread_to_join != currentThread->tid()){				
						DEBUG('t', "--Halt call: Joining with thread #%d...\n", thread_to_join);
						currentThread->join(thread_to_join);
						DEBUG('t', "--Halt call: Thread #%d has joined\n", thread_to_join);
						e = currentThread->space->threadList()->getFirst(); // To be sure that we don't have a de-allocated 'e'
					}
				} while ((e = e->next) && currentThread->space->countThread() > 1);
				interrupt->Halt();
				break;
			}

			case SC_Exit: {
				DEBUG('i', "Exit syscall, initiated by user program.\n");
				/*! \todo Handle correct process exit  */
				//currentThread->Finish();
				//For now just call Halt()
				interrupt->Halt();
				break;
			}

			case SC_Yield: {
				DEBUG('i', "Current thread #%d nicely ask to yield.\n", currentThread->tid());
				currentThread->Yield();
				break;
			}

			case SC_PutChar: {
				DEBUG('i', "PutChar syscall, initiated by user program.\n");
				currentThread->space->acquireIO();
				synchconsole->PutChar((char) reg4);
				currentThread->space->releaseIO();
				break;
			}

			case SC_GetChar: {
				DEBUG('i', "GetChar syscall, initiated by user program.\n");
				currentThread->space->acquireIO();
				returnvalue = synchconsole->GetChar();
				DEBUG('i', "GetChar syscall, returned.\n");
				machine->WriteRegister(2, returnvalue);
				currentThread->space->releaseIO();
				break;
			}

			case SC_PutString: {
				DEBUG('i', "PutString syscall, initiated by user program.\n");
				currentThread->space->acquireIO();
				char* buffer = copyStringFromMachine(reg4, (unsigned int)MAX_STRING_SIZE);
				synchconsole->PutString(buffer);
				free(buffer);
				currentThread->space->releaseIO();
				break;
			}

			case SC_GetString: {
				DEBUG('i', "GetString syscall, initiated by user program.\n");
				currentThread->space->acquireIO();
				char *buffer = (char *) malloc(sizeof(char) * MAX_STRING_SIZE);
				synchconsole->GetString(buffer, reg5);
				copyStringToMachine(buffer, reg4, MIN(MAX_STRING_SIZE, reg5));
				currentThread->space->releaseIO();
				break;
			}

			case SC_PutInt: {
				DEBUG('i', "PutInt syscall, initiated by user program.\n");
				currentThread->space->acquireIO();
				char *buffer = (char *) malloc(sizeof(char) * 11);

				snprintf(buffer, 11, "%d", reg4); /*! \todo make a real function if time availbale */
				synchconsole->PutString(buffer);

				free(buffer);
				currentThread->space->releaseIO();
				break;
			}

			case SC_GetInt: {
				DEBUG('i', "GetInt syscall, initiated by user program.\n");
				currentThread->space->acquireIO();
				char *buffer = (char *) malloc(sizeof(char) * 11);

				synchconsole->GetString(buffer, 11);
				sscanf(buffer, "%d", &returnvalue); /*! \todo make a real function if time availbale */
				machine->WriteMem(reg4, 11, returnvalue);

				free(buffer);
				currentThread->space->releaseIO();
				break;
			}

			case SC_CreateUserThread: {
				DEBUG('i', "CreateUserThread syscall, initiated by user program.\n");
				returnvalue = do_UserThreadCreate(reg4, reg5, reg6);
				machine->WriteRegister(2, returnvalue);
				break;
			}

			case SC_ExitUserThread: {
				DEBUG('i', "ExitUserThread syscall, initiated by user program.\n");
				do_UserThreadExit();
				break;
			}

			case SC_JoinUserThread: {
				DEBUG('i', "UserThreadJoin syscall, initiated by user program.\n");
				returnvalue = (int)do_UserThreadJoin((tid_t)reg4);
				machine->WriteRegister(2, returnvalue);
				break;
			}

			case SC_SemaInit: {
				DEBUG('i', "SemaInit syscall, initiated by user program.\n");
				*(void**)(machine->mainMemory + reg4) = new Semaphore("User Semaphore", reg5);
				break;
			}

			case SC_SemaPost: {
				DEBUG('i', "SemaPost syscall, initiated by user program.\n");
				((Semaphore*)reg4)->V();
				break;
			}

			case SC_SemaWait: {
				DEBUG('i', "SemaWait syscall, initiated by user program.\n");
				((Semaphore*)reg4)->P();
				break;
			}
			
			case SC_ForkExec: {
				DEBUG('i', "ForkExec syscall, initiated by user program.\n");
				
				break;
			}

			default: {
				printf("Unexpected user mode exception %d %d\n", which, type);
				ASSERT(FALSE);
			}
		}

		// LB: Do not forget to increment the pc before returning!
		UpdatePC ();
		// End of addition
	}

}
