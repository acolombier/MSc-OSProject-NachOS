/*! addrspace.h 
//      Data structures to keep track of executing user programs 
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "system.h"
#include "synch.h"

/*!
 * \def UserStackSize Used macro for stack allocation
 */
#define UserStackSize		1024	// increase this as necessary!

class Thread;

class AddrSpace
{
  public:
    AddrSpace (OpenFile * executable);	// Create an address space,
    // initializing it with the program
    // stored in the file "executable"
    ~AddrSpace ();		// De-allocate an address space

    void InitRegisters ();	// Initialize user-level CPU registers,
    // before jumping to user code

    void SaveState ();		// Save/restore address space-specific
    void RestoreState ();	// info on a context switch 
    
    Thread* getThread(unsigned int tid);
	inline List* threadList() { return mThreadList; }
    
    void appendThread(Thread*);
    void removeThread(Thread*);
    
    inline unsigned int countThread() const { return mThreadList->size(); }
    
    inline void acquireIO() { mIOLock->Acquire(); }
    inline void releaseIO() { mIOLock->Release(); }

  private:
      TranslationEntry * pageTable;	// Assume linear page table translation
    // for now!
    unsigned int numPages;	// Number of pages in the virtual 
    tid_t lastTID;
    List* mThreadList;
    
    Lock* mIOLock;
    // address space
};

#endif // ADDRSPACE_H
