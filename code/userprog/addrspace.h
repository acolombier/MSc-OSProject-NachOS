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

#include <exception>

/*!
 * \def UserStackSize Used macro for stack allocation
 */
#define UserStackSize        512    // increase this as necessary!

class Thread;

extern "C" {
    typedef struct thread_bundle_struct {
        Thread* object;
        int result_code;
        tid_t tid;
        unsigned int ref_cnt;
    } thread_bundle_t;
    
    typedef struct addrspace_bundle_struct {
        AddrSpace* object;
        int result_code;
        SpaceId pid;
        unsigned int ref_cnt;
    } addrspace_bundle_t;
    
    typedef struct fd_bundle_struct {
        char* pathname;
        void* object;
        /*int type; if we want to handle socket on the same way */
        int fd;
    } fd_bundle_t;
}

class PermissionException : public std::exception {
};

class ExecutableException : public std::exception {
};

class AddrSpace
{
  public:
    AddrSpace (OpenFile * executable);    // Create an address space,
    // initializing it with the program
    // stored in the file "executable"
    ~AddrSpace ();        // De-allocate an address space

    void InitRegisters ();    // Initialize user-level CPU registers,
    // before jumping to user code

    void SaveState ();        // Save/restore address space-specific
    void RestoreState ();    // info on a context switch

    Thread* getThread(unsigned int tid);
    inline List* threadList() { return mThreadList; }

    void appendThread(Thread*);
    void removeThread(Thread*, int result_code);

    /*!
     * Modify the number of page allocated between the code and the stack
     * \param n the number used for the brk shift
     */
    int Sbrk(int n);
    
    unsigned int countThread() const;
    
    inline SpaceId pid() const { return mPid; }
    inline SpaceId ppid() const { return mPpid; }
    
    inline void ppid(SpaceId arg_pid) { mPpid = arg_pid; }
    
    static unsigned int ADDR_SPACE_COUNT();
    
    static addrspace_bundle_t* INC_REF(SpaceId);
    static void DEC_REF(SpaceId);

    /*!
     * Append a Thread waiting for this process to finish
     * \param t The Thread which is sleeping, waiting to this one to finish
     */
    void appendToJoin(Thread*t);
    
    /*!
     * Join the current process, with the other process
     * \param pid The process to join
     * \param result_code_pnt the pointer where to store the result code (optionnal)
     */
    int join(SpaceId pid, int result_code_pnt = 0);
    
    /*!
     * Increase the reference counter of the bundle
     * \param t The thread TID
     * \return the bundle it refers to
     */
    thread_bundle_t* inc_ref_thread(tid_t tid);
    
    /*!
     * Decrease the reference counter of the bundle
     * \param t The thread TID
     */
    void dec_ref_thread(tid_t pid);
    
    /*!
     * Return the fd bundle for a given fd. Null if not found
     * \param fd The thread TID
     * \return a bundle pointer
     */
    fd_bundle_t* get_fd(int fd);
    
    /*!
     * Append a new bundle to the addrspace.
     * \param bundle the fd bundle
     * \return the fd. -1 if error
     */
    int store_fd(fd_bundle_t*);
    
    /*!
     * Remove a bundle from the addrspace.
     * Careful! It does not handle the fs close!
     * 
     * \param bundle the fd bundle
     */
    void del_fd(int fd);

  private:
    TranslationEntry * pageTable;    // Assume linear page table translation
    int stackUsage[MAX_THREADS];
    // for now!
    unsigned int numPages;    // Number of pages in the virtual
    tid_t lastTID;
    int lastFD;
    List* mThreadList;

    unsigned int mBrk;

    SpaceId mPid;
    SpaceId mPpid;
    List* mThreadsWaiting;
    
    fd_bundle_t* mFdTable; 
    Lock* fd_lock;

  protected:
    static Lock* _ADDR_SPACE_LOCK;

    static SpaceId _LAST_PID;
    static List* _SPACE_LIST;
    
    // address space
};

#endif // ADDRSPACE_H
