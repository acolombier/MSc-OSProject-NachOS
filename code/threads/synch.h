// synch.h
//      Data structures for synchronizing threads.
//
//      Three kinds of synchronization are defined here: semaphores,
//      locks, and condition variables.  The implementation for
//      semaphores is given; for the latter two, only the procedure
//      interface is given -- they are to be implemented as part of
//      the first assignment.
//
//      Note that all the synchronization objects take a "name" as
//      part of the initialization.  This is solely for debugging purposes.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// synch.h -- synchronization primitives.

#ifndef SYNCH_H
#define SYNCH_H

#include "copyright.h"
#include "list.h"

class Thread;

// The following class defines a "semaphore" whose value is a non-negative
// integer.  The semaphore has only two operations P() and V():
//
//      P() -- waits until value > 0, then decrement
//
//      V() -- increment, waking up a thread waiting in P() if necessary
//
// Note that the interface does *not* allow a thread to read the value of
// the semaphore directly -- even if you did read the value, the
// only thing you would know is what the value used to be.  You don't
// know what the value is now, because by the time you get the value
// into a register, a context switch might have occurred,
// and some other thread might have called P or V, so the true value might
// now be different.

class Semaphore
{
  public:
    Semaphore (const char *debugName, int initialValue);// set initial value
     ~Semaphore ();		// de-allocate semaphore
    const char *getName ()
    {
	return name;
    }				// debugging assist

    void P ();			// these are the only operations on a semaphore
    void V ();			// they are both *atomic*

	//----------------------------------------------------------------------
	// Semaphore::E
	//      Return the internal value of the samphore
	//----------------------------------------------------------------------

    inline int E () const {return value;}			// they are both *atomic*

  private:
    const char *name;		// useful for debugging
    int value;			// semaphore value, always >= 0
    List *queue;		// threads waiting in P() for the value to be > 0
};

// The following class defines a "lock".  A lock can be BUSY or FREE.
// There are only two operations allowed on a lock:
//
//      Acquire -- wait until the lock is FREE, then set it to BUSY
//
//      Release -- set lock to be FREE, waking up a thread waiting
//              in Acquire if necessary
//
// In addition, by convention, only the thread that acquired the lock
// may release it.  As with semaphores, you can't read the lock value
// (because the value might change immediately after you read it).

/*! A lock mechanism made on top of the semaphore mechanism provided. */
/*!
 * This Lock class is used to create a exclusion mechnaism in order to ensure
 * MUTual EXclusion (the so called mutex on Linux system).
 */
class Lock
{
  public:
    /*! The constructor of a Lock object. */
    /*!
     * \param debugName The name displayed when debug trace is enabled.
     */
    Lock(const char *debugName);  // initialize lock to be FREE

    /*! The destructor of a Lock object */
    ~Lock();

    /*! Get the debugName of the Lock object. */
    /*!
     * \return Return the debugName of the Lock.
     */
    inline const char *getName() const { return mName; }  // debugging assist

    /*! Blocks the calling Thread until the Thread obtains ownership of the mutex (atomic operation). */
    void Acquire();
    /*! Releases ownership of the mutex (atomic operation). */
    void Release();

    /*! Indicate if currentThread is currently blocking this Lock. */
    /*!
     * \return Return true if the currentThread is the owner of this Lock.
     */
    bool isHeldByCurrentThread() const;
    // Useful for checking in Release, and in Condition variable ops below.

  private:
    const char *mName;  // for debugging plus some other stuff you'll need to define
    Thread* mThreadHolder;
    Semaphore* mMutexLock;
};

// The following class defines a "condition variable".  A condition
// variable does not have a value, but threads may be queued, waiting
// on the variable.  These are only operations on a condition variable:
//
//      Wait() -- release the lock, relinquish the CPU until signaled,
//              then re-acquire the lock
//
//      Signal() -- wake up a thread, if there are any waiting on
//              the condition
//
//      Broadcast() -- wake up all threads waiting on the condition
//
// All operations on a condition variable must be made while
// the current thread has acquired a lock.  Indeed, all accesses
// to a given condition variable must be protected by the same lock.
// In other words, mutual exclusion must be enforced among threads calling
// the condition variable operations.
//
// In Nachos, condition variables are assumed to obey *Mesa*-style
// semantics.  When a Signal or Broadcast wakes up another thread,
// it simply puts the thread on the ready list, and it is the responsibility
// of the woken thread to re-acquire the lock (this re-acquire is
// taken care of within Wait()).  By contrast, some define condition
// variables according to *Hoare*-style semantics -- where the signalling
// thread gives up control over the lock and the CPU to the woken thread,
// which runs immediately and gives back control over the lock to the
// signaller when the woken thread leaves the critical section.
//
// The consequence of using Mesa-style semantics is that some other thread
// can acquire the lock, and change data structures, before the woken
// thread gets a chance to run.

/*! A variable condition mechanism made on top of the lock mechanism. */
/*!
 * This Condition class is used to create syncronisation between threads such as
 * the 'cond' mechanism in Linux.
 * \todo Made out of semaphore, patch to use lock.
 */
class Condition
{
  public:
    /*! The constructor of a Condition object. */
    /*!
     * \param debugName the name displayed when debug trace is enabled.
     */
    Condition(const char *debugName);  // initialize condition to "no one waiting"

    /*! The destructor of a Condition object. */
    ~Condition();  // deallocate the condition

    /*! Get the debugName of the Condition object. */
    /*!
     * \return Return the debugName of the Condition.
     */
    inline const char *getName() const { return mName; }

    /*! Blocks a thread (atomic operation). */
    /*!
     * \param conditionLock The Lock used by the Condition object. The calling Thread *must* own this Lock.
     */
    void Wait(Lock *conditionLock);

    /*! Unblocks one of the threads that are waiting for the condition_variable object (atomic operation). */
    /*!
     * \param conditionLock The Lock used by the Condition object. The calling Thread *must* own this Lock.
     */
    void Signal(Lock *conditionLock);

    /*! Unblocks all threads that are waiting on the specified lock (atomic operation). */
    /*!
     * \param conditionLock The Lock used by the Condition object. The calling Thread *must* own this Lock.
     */
    void Broadcast(Lock *conditionLock);

  private:
    const char *mName;
    List* mThreadsQueue;
    // plus some other stuff you'll need to define
};
#endif // SYNCH_H
