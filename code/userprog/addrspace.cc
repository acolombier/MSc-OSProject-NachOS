/*! 
//      Routines to manage address spaces (executing user programs).
//
//      In order to run a user program, you must:
//
//      1. link with the -N -T 0 option 
//      2. run coff2noff to convert the object file to Nachos format
//              (Nachos object code format is essentially just a simpler
//              version of the UNIX executable object code format)
//      3. load the NOFF file into the Nachos file system
//              (if you haven't implemented the file system yet, you
//              don't need to do this last step)
 */
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "openfile.h"

#include <strings.h>		/* for bzero */

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the 
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader * noffH)
{
    noffH->noffMagic = WordToHost (noffH->noffMagic);
    noffH->code.size = WordToHost (noffH->code.size);
    noffH->code.virtualAddr = WordToHost (noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost (noffH->code.inFileAddr);
    noffH->initData.size = WordToHost (noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost (noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost (noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost (noffH->uninitData.size);
    noffH->uninitData.virtualAddr =
	WordToHost (noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost (noffH->uninitData.inFileAddr);
}

/*!
 * ReadAtVirtual
 * \param executable executable
 * \param into virtual address MIPS pointer
 * \param numBytes size to read
 * \param position offset
 * \param *pageTable table used for writing
 * \param numPages page
 * \return size actually read
 */
static void ReadAtVirtual(OpenFile *executable, int virtualaddr,
	int numBytes, int position,
	TranslationEntry *pageTable,unsigned numPages){
    char* buf = new char[numBytes];
    numBytes = executable->ReadAt(buf, numBytes, position);
    
    for (int i = 0; i < numBytes; i++)
		machine->WriteMem(virtualaddr + i, 1, buf[i]);
		
    delete [] buf;
}

SpaceId AddrSpace::_LAST_PID = 0;
List* AddrSpace::_SPACE_LIST = new List();
Lock* AddrSpace::_ADDR_SPACE_LOCK = new Lock("AddrSpace");

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      First, set up the translation from program memory to physical 
//      memory.  For now, this is really simple (1:1), since we are
//      only uniprogramming, and we have a single unsegmented page table
//
//      "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace (OpenFile * executable):
	lastTID(0), mThreadList(new List), mThreadsWaiting(new List)
{
	AddrSpace::_ADDR_SPACE_LOCK->Acquire();
	mPid = ++AddrSpace::_LAST_PID;
	AddrSpace::_SPACE_LIST->Append(this);
	
	ListElement* e = AddrSpace::_SPACE_LIST->getFirst();
	
	do {
		DEBUG('c', "ListProcess: current is %d\n", ((AddrSpace*)e->item)->pid());
	} while ((e = e->next));
	AddrSpace::_ADDR_SPACE_LOCK->Release();
	
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt ((char *) &noffH, sizeof (noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
	(WordToHost (noffH.noffMagic) == NOFFMAGIC))
	SwapHeader (&noffH);
    ASSERT (noffH.noffMagic == NOFFMAGIC);

	// how big is address space?
    //~ size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize * MAX_THREADS;	// we need to increase the size
    // to leave room for the stack

    ASSERT (numPages <= NumPhysPages);	// check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    DEBUG ('a', "Initializing address space, num pages %d, size %d\n",
	   numPages, size);

	// first, set up the translation 
    pageTable = new TranslationEntry[ADDRSPACE_PAGES_SIZE];

	int ro_code = divRoundUp(noffH.code.size + noffH.initData.size, PageSize),
		rw_code = divRoundUp(noffH.uninitData.size, PageSize);
    
    for (i = 0; i < ADDRSPACE_PAGES_SIZE; i++)
		pageTable[i].virtualPage(i);
		
    for (i = 0; i < ro_code; i++){
		pageTable[i].setReadOnly(true);
		pageTable[i].setValid(true);
		pageTable[i].physicalPage(frameprovider->GetEmptyFrame());
	}
    for (; i < rw_code; i++){
		pageTable[i].setValid(true);
		pageTable[i].physicalPage(frameprovider->GetEmptyFrame());
	}
	
    numPages = ro_code + rw_code;
    size = numPages * PageSize;
    mBrk = size - 1;
    
    RestoreState();

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
		DEBUG ('a', "Initializing code segment, at 0x%x, size %d\n",
		noffH.code.virtualAddr, noffH.code.size);
		
		ReadAtVirtual(executable, noffH.code.virtualAddr, noffH.code.size,
				noffH.code.inFileAddr, pageTable, numPages);
    }
    if (noffH.initData.size > 0) {
		DEBUG ('a', "Initializing data segment, at 0x%x, size %d\n",
		noffH.initData.virtualAddr, noffH.initData.size);
		
		ReadAtVirtual(executable, noffH.initData.virtualAddr, noffH.initData.size,
				noffH.initData.inFileAddr, pageTable, numPages);
   }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//      Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace ()
{
  // LB: Missing [] for delete
  // delete pageTable;
	ASSERT(countThread() == 0);
	
    for (unsigned int i = 0; i < ADDRSPACE_PAGES_SIZE; i++)
		if (pageTable[i].valid())
			frameprovider->ReleaseFrame(pageTable[i].physicalPage());
			
	DEBUG('a', "Deleting pageTable after releasing frame.\n");		
	delete [] pageTable;

	// Waking up waiting threads from other process
	DEBUG ('t', "%d thread(s) were waiting this space to finish, notifying...\n", mThreadsWaiting->size());
    Thread* t;
    while ((t = (Thread*)mThreadsWaiting->Remove())){
		DEBUG ('t', "Waking up thread %s#%d from process %d...\n", t->getName(), t->tid(), t->space->pid());
		scheduler->ReadyToRun(t);
	}
	
	DEBUG('a', "Deleting list of waiting threads after waking them up.\n");		
	delete mThreadsWaiting;
	
	DEBUG('a', "Deleting list of threads as no one is here anymore.\n");		
	delete mThreadList;
	AddrSpace::_ADDR_SPACE_LOCK->Acquire();
	DEBUG('a', "Removing process %d of the registered process list\n", pid());
	AddrSpace::_SPACE_LIST->Remove(this);
	AddrSpace::_ADDR_SPACE_LOCK->Release();
  // End of modification
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//      Set the initial values for the user-level register set.
//
//      We write these directly into the "machine" registers, so
//      that we can immediately jump to user code.  Note that these
//      will be saved/restored into the currentThread->userRegisters
//      when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters ()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister (i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister (PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister (NextPCReg, 4);

    ASSERT(MAX_THREADS >= countThread());
    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    
    machine->WriteRegister (StackReg, (numPages - countThread() + 1) * PageSize - 16);
    DEBUG ('a', "Initializing stack register to %d for thread #%d\n",
	   (numPages - countThread() + 1) * PageSize - 16, currentThread->tid());
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//      On a context switch, save any machine state, specific
//      to this address space, that needs saving.
//
//      For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState (){
}
	
/*!
 * Add a Thread to the address space
 * 
 * \param t freshly created thread
 * 
 */
void AddrSpace::appendThread (Thread* t){	
	stackNbPages = divRoundUp(UserStackSize, PageSize);
	if (countThread() < MAX_THREADS && mBrk >= (ADDRSPACE_PAGES_SIZE * PageSize) - (UserStackSize * (countThread() + 1)) && frameprovider->GetEmptyFrame() >= stackNbPages){
		t->space = this;
		t->setTID(lastTID++);
		mThreadList->Append(t);
		DEBUG ('a', "New thread mapped in the space as ID #%d\n", t->tid());

		for (i = 0; i < stackNbPages; i++){
			pageTable[ADDRSPACE_PAGES_SIZE - i - (stackNbPages * (countThread() - 1) - 1].setValid(true);
			pageTable[ADDRSPACE_PAGES_SIZE - i - (stackNbPages * (countThread() - 1) - 1].physicalPage(frameprovider->GetEmptyFrame());
		}
		numPages += stackNbPages;
		size = numPages * PageSize;
		DEBUG ('a', "Stack allocated for thread ID #%d\n", t->tid());
	} else if (countThread() < MAX_THREADS)
		DEBUG ('a', "Maximun threads number reached\n");
	else
		DEBUG ('a', "No more page free to hold a thread stack\n");
}
/*!
 * Remove a Thread to the address space, and notify the waiting Threads
 * 
 * \param t freshly created thread
 * 
 * */
void AddrSpace::removeThread(Thread* t){
	char found = mThreadList->Remove(t);
	DEBUG('t', "Thread #%d has %sbeen found\n", t->tid(), (found ? "": "NOT "));
}

/*!
 * Append a Thread from another space, and notify the waiting Threads when it will be deleted
 * 
 * \param t the waiting thread
 * 
 * */
void AddrSpace::appendToJoin(Thread*t) {
	mThreadsWaiting->Append(t);
}


/*!
 * Block the current thread related to this space, until the given AddrSpace is deleted
 * 
 * \param pid The space ID
 * 
 * */
int AddrSpace::join(SpaceId s_pid) {
#ifdef USER_PROGRAM		
	ASSERT(pid() != s_pid);
	ASSERT(currentThread->space == this);
	
	AddrSpace::_ADDR_SPACE_LOCK->Acquire();
	
	ListElement* e = AddrSpace::_SPACE_LIST->getFirst();
	while (s_pid != ((AddrSpace*)e->item)->pid() && (e = e->next)) {}
	AddrSpace::_ADDR_SPACE_LOCK->Release();

	if (!e || !e->item){
		DEBUG('c', "JoiningProcess: %d does not exist\n", s_pid);
		return 0;
	}
		
	AddrSpace* process_to_join = (AddrSpace*)e->item;
		
	process_to_join->appendToJoin(currentThread); // Will be elected when the thread will finish
	
    IntStatus oldLevel = interrupt->SetLevel (IntOff);	// disable interrupts
	currentThread->Sleep ();
    (void) interrupt->SetLevel (oldLevel);	// re-enable interrupts
#endif
	return 0;
}



/*!
 * Get a thread by is ID on the space
 * 
 * \param tid the thread id
 * \return the thread matching with this id
 * 
 * */
Thread* AddrSpace::getThread(unsigned int tid) {
	ListElement* e = mThreadList->getFirst();
	while ((e = e->next) && tid != ((Thread*)e->item)->tid()) {}
	return (e ? (Thread*)e->item : nullptr);
}


/*!
 * Allocate new page to a process
 * 
 * \param the number of page required
 * \return a boolean saying wheather there is an error (false is success)
 * 
 * */
int AddrSpace::Sbrk(unsigned int n){
	stackNbPages = divRoundUp(UserStackSize);

	stackStart = (ADDRSPACE_PAGES_SIZE - (stackNbPages * countThread())) * PageSize;
	
	if (mBrk + (n * PageSize) >= stackStart){
		DEBUG('a', "Trying to allocate new pages, but none available in the address space.\n");
		return NULL;
	}

	if (frameprovider->NumAvailFrame() < n){
		DEBUG('a', "Trying to allocate new pages, but none available in the physical memory.\n");
		return NULL;
	}

	ASSERT(divRoundDown(mBrk, PageSize) == divRoundUp(mBrk, PageSize));
	
	for (i = divRoundDown(mBrk, PageSize); i < divRoundDown(mBrk, PageSize) + n; i++){
		pageTable[i].setValid(true);
		pageTable[i].physicalPage(frameprovider->GetEmptyFrame());
	}
	int oldBrk = mBrk;
	mBrk += n * PageSize;
	return oldBrk;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//      On a context switch, restore the machine state so that
//      this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void
AddrSpace::RestoreState ()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
