/*! \file system.h
*   \brief All global variables used in Nachos are defined here.
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

/*! \def MAX_STRING_SIZE

    The maximun string size that the kernel can handle. By default 512, including the \0 char.
*/
#define MAX_STRING_SIZE 512

/*! \def MAX_THREADS

    The maximun number of userthread that can be associated to an address space (= process) .
*/
#define MAX_THREADS 16

/*! \def ADDRSPACE_PAGES_SIZE

    The number of page that a virtual space contains by default.
*/
#define ADDRSPACE_PAGES_SIZE 128

#ifndef SYSTEM_H
#define SYSTEM_H

typedef unsigned int tid_t;
typedef int SpaceId;

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"


// Initialization and cleanup routines
extern void Initialize (int argc, char **argv);	// Initialization,
						// called before anything else
extern void Cleanup ();		// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;	// the thread holding the CPU
extern Thread *threadToBeDestroyed;	// the thread that just finished
extern Scheduler *scheduler;	// the ready list
extern Interrupt *interrupt;	// interrupt status
extern Statistics *stats;	// performance metrics
extern Timer *timer;		// the hardware alarm clock

#ifdef USER_PROGRAM

#include "machine.h"
extern Machine *machine;	// user program memory and registers
#include "synchconsole.h"
extern SynchConsole *synchconsole;		// synchronized console
#include "frameprovider.h"
extern FrameProvider *frameprovider;		// synchronized console
#endif

#ifdef FILESYS_NEEDED		// FILESYS or FILESYS_STUB
#include "filesys.h"
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice *postOffice;
#endif

#endif // SYSTEM_H
