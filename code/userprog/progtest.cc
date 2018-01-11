// progtest.cc 
//      Test routines for demonstrating that Nachos can load
//      a user program and execute it.  
//
//      Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "synchconsole.h"
#include "addrspace.h"
#include "synch.h"

#ifdef TRACK_ALLOCATION
typedef struct alloc_history_struct {
	void* ptr;
	size_t size;
	char has_been_deleted;
} alloc_hist_t;

alloc_hist_t** history = nullptr;
int history_size = 0;

void* operator new(size_t num)
{
    DEBUG('l', "Requesting allocation of %d bytes\n", num);
	void* ptr = malloc(num);
    DEBUG('l', "Allocating %d bytes at %p\n", num, ptr);
    
    int i;
    for (i = 0; i < history_size; i++)
		if (history[i]->ptr == ptr)
			break;
	if (i == history_size){
		history = (alloc_hist_t**)realloc(history, (history_size + 1) * sizeof(alloc_hist_t*));
		history[history_size] = (alloc_hist_t*)malloc(sizeof(alloc_hist_t));
		i = history_size++;
	}
    history[i]->ptr = ptr;
    history[i]->size = num;
    history[i++]->has_been_deleted = 0;
    return ptr;
}
 
void operator delete(void *ptr)
{
    DEBUG('l', "Deallocating %p...\n", ptr);
    int i;
    for (i = 0; i < history_size; i++)
		if (history[i]->ptr == ptr)
			break;

	ASSERT(i < history_size);

	
    DEBUG('l', "Record found: %p with size %d\n", history[i]->ptr, history[i]->size);
	if (history[i]->has_been_deleted)
		ASSERT(FALSE);
    free(ptr);
    history[i]->has_been_deleted = 1;
}
#endif

//----------------------------------------------------------------------
// StartProcess
//      Run a user program.  Open the executable, load it into
//      memory, and jump to it.
//----------------------------------------------------------------------
void
StartProcess (char *filename)
{
    OpenFile *executable = fileSystem->Open (filename);
    AddrSpace *space;

    if (executable == NULL){
		printf ("Unable to open file %s\n", filename);
		return;
	}
    space = new AddrSpace (executable);
    
    fileSystem->Close (executable);		// close file
    
    space->appendThread(currentThread);
    
    DEBUG('t', "The main thread is #%d\n", currentThread->tid());

    space->InitRegisters ();	// set the initial register values
    space->RestoreState ();	// load page table register

    machine->Run ();		// jump to the user progam
    ASSERT (FALSE);		// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
//      Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void
ReadAvail (int arg)
{
    readAvail->V ();
}
static void
WriteDone (int arg)
{
    writeDone->V ();
}

//----------------------------------------------------------------------
// ConsoleTest
//      Test the console by echoing characters typed at the input onto
//      the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console (in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore ("read avail", 0);
    writeDone = new Semaphore ("write done", 0);

    for (;;)
      {
	  readAvail->P ();	// wait for character to arrive
	  ch = console->GetChar ();
	  console->PutChar (ch);	// echo it!
	  writeDone->P ();	// wait for write to finish
	  if (ch == 'q')
	      return;		// if q, quit
      }
}
//----------------------------------------------------------------------
// SyncConsoleTest
//      Test the console by echoing characters typed at the input onto
//      the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void
SynchConsoleTest (char *in, char *out)
{
    char ch;

    console = new SynchConsole (in, out);

    for ( ; ch != 'q'; ch = console->GetChar ())
	  console->PutChar (ch);	// echo it!
	  
	return;
}
