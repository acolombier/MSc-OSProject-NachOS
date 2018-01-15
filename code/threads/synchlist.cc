// synchlist.cc
//      Routines for synchronized access to a list.
//
//      Implemented by surrounding the List abstraction
//      with synchronization routines.
//
//      Implemented in "monitor"-style -- surround each procedure with a
//      lock acquire and release pair, using condition signal and wait for
//      synchronization.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchlist.h"
#include "interrupt.h"
#include "system.h"

//----------------------------------------------------------------------
// SynchList::SynchList
//      Allocate and initialize the data structures needed for a
//      synchronized list, empty to start with.
//      Elements can now be added to the list.
//----------------------------------------------------------------------

SynchList::SynchList ()
{
    list = new List ();
    lock = new Lock ("list lock");
    listEmpty = new Condition ("list empty cond");
    hasTimeout = false;
}

//----------------------------------------------------------------------
// SynchList::~SynchList
//      De-allocate the data structures created for synchronizing a list.
//----------------------------------------------------------------------

SynchList::~SynchList ()
{
    delete list;
    delete lock;
    delete listEmpty;
}

//----------------------------------------------------------------------
// SynchList::Append
//      Append an "item" to the end of the list.  Wake up anyone
//      waiting for an element to be appended.
//
//      "item" is the thing to put on the list, it can be a pointer to
//              anything.
//----------------------------------------------------------------------

void
SynchList::Append (void *item)
{
    lock->Acquire ();		// enforce mutual exclusive access to the list
    list->Append (item);
    listEmpty->Signal (lock);	// wake up a waiter, if any
    lock->Release ();
}

//----------------------------------------------------------------------
// SynchList::Remove
//      Remove an "item" from the beginning of the list.  Wait if
//      the list is empty.
// Returns:
//      The removed item.
//----------------------------------------------------------------------

void *
SynchList::Remove (int timeout)
{
    void *item;

    lock->Acquire ();		// enforce mutual exclusion

    if (timeout > -1)
        interrupt->Schedule(TimeoutHandler, (int) this, timeout, IntType::TimerInt);

    while (list->IsEmpty () && !(timeout != -1 && hasTimeout))
        //DEBUG('n', "%d && !(%d && %d)", list->IsEmpty (), timeout == -1, hasTimeout);
        listEmpty->Wait (lock);	// wait until list isn't empty

    if (hasTimeout) {
        item = NULL;
        hasTimeout = false;
    } else {
        item = list->Remove ();
        ASSERT (item != NULL);
    }
    lock->Release ();
    return item;
}

//----------------------------------------------------------------------
// SynchList::Mapcar
//      Apply function to every item on the list.  Obey mutual exclusion
//      constraints.
//
//      "func" is the procedure to be applied.
//----------------------------------------------------------------------

void
SynchList::Mapcar (VoidFunctionPtr func)
{
    lock->Acquire ();
    list->Mapcar (func);
    lock->Release ();
}

//----------------------------------------------------------------------
// SynchList::Unlock
//
//----------------------------------------------------------------------

void
SynchList::Unlock ()
{
    lock->Acquire ();
    hasTimeout = true;
    listEmpty->Broadcast (lock);
    lock->Release ();
}


void
SynchList::TimeoutHandler (int synchL)
{
    DEBUG('n', "SynchList had a timeout. Yikes.\n");
    ((SynchList *)synchL)->Unlock ();
}
