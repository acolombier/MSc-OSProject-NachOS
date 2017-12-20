// threadtest.cc 
//      Simple test case for the threads assignment.
//
//      Create two threads, and have them context switch
//      back and forth between themselves by calling Thread::Yield, 
//      to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// SimpleThread
//      Loop 5 times, yielding the CPU to another ready thread 
//      each iteration.
//
//      "which" is simply a number identifying the thread, for debugging
//      purposes.
//----------------------------------------------------------------------

void
SimpleThread (int which)
{
    int num;

    for (num = 0; num < 5; num++)
      {
	  printf ("*** thread %d looped %d times\n", which, num);
	  currentThread->Yield ();
      }
}



//----------------------------------------------------------------------
// ThreadTest
//      Set up a ping-pong between two threads, by forking a thread 
//      to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest ()
{
    DEBUG ('t', "Entering SimpleTest\n");

    Thread *t = new Thread ("forked thread");

    t->Fork (SimpleThread, 1);
    SimpleThread (0);
    
}

// EXTRA

/* ************************* Testing Locks ************************** */
#include "synch.h"
int count = 0;
Lock *lock = new Lock("mutex");

//----------------------------------------------------------------------
// SimpleIncrement
//      Loop 5 times incrementing a shared variable.
//      The synchronisation is done with the lock class
//
//      "which" is simply a number identifying the thread, for debugging
//      purposes.
//----------------------------------------------------------------------

void
SimpleIncrement (int which)
{
    int num;

    for (num = 0; num < 5; num++)
      {
	  //printf("Thread %d trying to lock\n",which);
	  lock->Acquire();
	  //printf("Thread %d has locked\n",which);
	  
	  count++;
	  printf("Thread %d : count = %d\n", which, count);

	  
	  //currentThread->Yield ();
	  
	  //printf("Thread %d trying to unlock\n",which);
	  lock->Release();
	  //printf("Thread %d has unlocked\n",which);
	  
	  currentThread->Yield ();
      }
}

//----------------------------------------------------------------------
// LockTest
//      Three threads are trying to incrment a counter 5 times each.
//      The synchonisation is done with locks.
//----------------------------------------------------------------------

void
LockTest ()
{
    
    DEBUG ('t', "Entering LockTest\n");

    printf("\nTesting the locks\n\n");
    
    Thread *t1 = new Thread ("forked thread 1");
    Thread *t2 = new Thread ("forked thread 2");
    
    t1->Fork (SimpleIncrement, 1);
    t2->Fork (SimpleIncrement, 2);
    SimpleIncrement (0);

    printf("\nFinal value of count : %d\n",count);
}
