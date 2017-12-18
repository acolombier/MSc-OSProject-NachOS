#include "system.h"
#include "userthreads.h"
#define PAGESPERTHREAD 10

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

static void StartUserThread (int f){

  DEBUG('t', "Inside start user thread\n");
  	int Fnaddress = (bundle* f->function);
  	int arg_list = (bundle*f->arg);
//TODO Remove after testing the control of program
  	int tid;
    tid=  currentThread->getThreadID();
    fprintf(stderr, "SC %d %d\n", Fnaddress,arg_list );
    fprintf(stderr, "Thread Execution  %d\n",tid );
    //upto this part
    currentThread->space->RestoreState();
    currentThread->space->InitRegisters();

  	machine->WriteRegister(4, arg_list);
  	machine->WriteRegister (PCReg,Fnaddress);
  	machine->WriteRegister (NextPCReg, Fnaddress + 4);
  	//int temp_stack_addr = currentThread->space->numPages ;
  	//temp_stack_addr = machine->ReadRegister(StackReg);
  	int pos;

  	pos = currentThread->space->GetStack();
  	if (pos == -1)
  	{
  		fprintf(stderr, "Insufficient Space for New Thread Creation \n" );
  		do_UserThreadExit();
  	}
  	currentThread->bitmapID= pos;

  	//fprintf(stderr, "%d\n", temp);
  	machine->WriteRegister (StackReg,  PAGESPERTHREAD *(pos+1) * PageSize);
  	//machine->WriteRegister (StackReg, currentThread->space->numPages*PageSize -16 - 3 * tid * PageSize);

  	// call this function to run the system
  	machine->Run();




  }
}
