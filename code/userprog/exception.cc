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
#include "userprocess.h"
#include "addrspace.h"
#include "directory.h"
#include "bitmap.h"

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

                do_UserHalt();
                break;
            }

            case SC_Exit: {
                DEBUG('c', "Exit syscall with code %d, initiated by user program.\n", reg4);
                do_UserProcessExit(reg4);
                break;
            }

            case SC_Join: {
                DEBUG('c', "Join syscall, initiated by user program.\n");
                returnvalue = (int)do_UserProcessJoin((SpaceId)reg4, reg5);
                machine->WriteRegister(2, returnvalue);
                break;
            }

            case SC_Yield: {
                DEBUG('c', "Current thread #%d nicely ask to yield.\n", currentThread->tid());
                currentThread->Yield();
                break;
            }

            case SC_PutChar: {
                DEBUG('c', "PutChar syscall, initiated by user program.\n");
                synchconsole->AcquireOutput();
                synchconsole->PutChar((char) reg4);
                synchconsole->ReleaseOutput();
                break;
            }

            case SC_GetChar: {
                DEBUG('c', "GetChar syscall, initiated by user program.\n");
                synchconsole->AcquireInput();
                returnvalue = synchconsole->GetChar();
                DEBUG('c', "GetChar syscall, returned.\n");
                machine->WriteRegister(2, returnvalue);
                synchconsole->ReleaseInput();
                break;
            }

            case SC_PutString: {
                DEBUG('c', "PutString syscall, initiated by user program.\n");
                char* buffer = copyStringFromMachine(reg4, (unsigned int)MAX_STRING_SIZE);
                synchconsole->AcquireOutput();
                synchconsole->PutString(buffer);
                delete [] buffer;
                synchconsole->ReleaseOutput();
                break;
            }

            case SC_GetString: {
                DEBUG('c', "GetString syscall, initiated by user program.\n");
                synchconsole->AcquireInput();
                char *buffer = (char *) malloc(sizeof(char) * MAX_STRING_SIZE);
                synchconsole->GetString(buffer, reg5);
                synchconsole->ReleaseInput();
                copyStringToMachine(buffer, reg4, MIN(MAX_STRING_SIZE, reg5));
                free(buffer);
                break;
            }

            case SC_PutInt: {
                DEBUG('c', "PutInt syscall, initiated by user program.\n");
                synchconsole->AcquireOutput();
                char *buffer = (char *) malloc(sizeof(char) * 11);

                snprintf(buffer, 11, "%d", reg4);
                synchconsole->PutString(buffer);

                free(buffer);
                synchconsole->ReleaseOutput();
                break;
            }

            case SC_GetInt: {
                DEBUG('c', "GetInt syscall, initiated by user program.\n");
                synchconsole->AcquireInput();
                char *buffer = (char *) malloc(sizeof(char) * 11);

                synchconsole->GetString(buffer, 11);
                sscanf(buffer, "%d", &returnvalue);

                free(buffer);
                synchconsole->ReleaseInput();
                machine->WriteMem(reg4, 4, returnvalue);
                break;
            }

            case SC_CreateUserThread: {
                DEBUG('c', "CreateUserThread syscall, initiated by user program.\n");
                returnvalue = do_UserThreadCreate(reg4, reg5, reg6);
                machine->WriteRegister(2, returnvalue);
                break;
            }

            case SC_ExitUserThread: {
                DEBUG('c', "ExitUserThread syscall, initiated by user program. %d thread(s) remaining in this space.\n", currentThread->space->countThread());

                if (currentThread->space->countThread() > 1)
                    do_UserThreadExit(reg4);
                else
                    do_UserProcessExit(reg4);
                break;
            }

            case SC_JoinUserThread: {
                DEBUG('c', "UserThreadJoin syscall, initiated by user program.\n");
                returnvalue = (int)do_UserThreadJoin((tid_t)reg4, reg5);
                machine->WriteRegister(2, returnvalue);
                break;
            }

            case SC_SemaInit: {
                int sema = (int)new Semaphore("User Semaphore", reg5);
                DEBUG('c', "SemaInit syscall on %p -> %p, initiated by user program.\n", reg4, (int*)sema);
                machine->WriteMem(reg4, 4, sema);
                break;
            }

            case SC_SemaPost: {
                DEBUG('c', "SemaPost syscall on %p, initiated by user program.\n", reg4);
                ((Semaphore*)reg4)->V();
                break;
            }

            case SC_SemaWait: {
                DEBUG('c', "SemaWait syscall on %p, initiated by user program.\n", reg4);
                ((Semaphore*)reg4)->P();
                break;
            }
            case SC_Kill: {
                DEBUG('c', "Kill syscall on %p, initiated by user program.\n", reg4);
                /*! \todo Implementation */
                break;
            }

            case SC_ForkExec: {
                DEBUG('c', "ForkExec syscall, initiated by user program.\n");

                char* filename = copyStringFromMachine(reg4, (unsigned int)MAX_STRING_SIZE);
                OpenFile *executable = fileSystem->Open (filename);
                
                if (executable == NULL){
                    DEBUG('c', "Unable to open file %s\n", filename);
                    delete [] filename;
                    machine->WriteRegister(2, 0);
                    break;
                } else
                    delete []filename;
                
                
                char** argv = nullptr;
                int argc = 0, arg_ptn;
                                
                if (reg5){
                    machine->ReadMem(reg5, 4, &arg_ptn);
                
                    DEBUG('c', "Fork has agument ");
                    while (arg_ptn){
                        argv = (char**)realloc(argv, (argc + 1) * sizeof(char*));
                        DEBUG('c', "%d:", arg_ptn);
                        argv[argc++] = copyStringFromMachine(arg_ptn, (unsigned int)MAX_STRING_SIZE);
                        DEBUG('c', "%s, ", argv[argc - 1]);
                        reg5 += 4;
                        machine->ReadMem(reg5, 4, &arg_ptn);
                    }
                    DEBUG('c', "\n");
                }
                
                returnvalue = do_UserProcessCreate(executable, argc, argv, reg6);
                machine->WriteRegister(2, returnvalue);            
                
                break;
            }
            
            case SC_Create: {
                DEBUG('c', "Create syscall with perm %d, initiated by user program.\n", reg5 >> 2);
                char* name = copyStringFromMachine(reg4, (unsigned int)MAX_STRING_SIZE);
                
                
                returnvalue = fileSystem->Create(name, 0, (FileHeader::Type)(reg5 & 0x3), (reg5 >> 2) & 0x7);
                if (!returnvalue){
                    OpenFile* object = fileSystem->Open(name);
                    if (object)
                        fileSystem->Close(object);
                    else
                        returnvalue = E_NOTFOUND;
                }
                delete [] name;
                
                machine->WriteRegister(2, returnvalue);
                break;
            }
            
            // FS Part
            case SC_Open: {
                DEBUG('c', "FileOpen syscall on %p, initiated by user program.\n", reg4);
                
                machine->WriteRegister(2, 0);    
                
                fd_bundle_t* b = new fd_bundle_t;
                b->pathname = copyStringFromMachine(reg4, (unsigned int)MAX_STRING_SIZE);
                b->object = fileSystem->Open(b->pathname);
                b->type = FileDescriptor;

                if (b->object == NULL){
                    DEBUG('c', "Unable to open file %s\n", b->pathname);
                    delete [] b->pathname;
                    delete b;
                    machine->WriteRegister(2, 0);                    
                    break;
                } else {
                    int fd = currentThread->space->store_fd(b);
                    if (fd)
                        machine->WriteRegister(2, fd);    
                    else {
                        fileSystem->Close((OpenFile*)b->object);
                        delete [] b->pathname;
                        delete b;
                    }
                }    
                
                break;
            }
            
            case SC_OpenDir: {
                DEBUG('c', "DirOpen syscall on %p, initiated by user program.\n", reg4);
                
                machine->WriteRegister(2, 0);    
                
                fd_bundle_t* b = new fd_bundle_t;
                b->pathname = copyStringFromMachine(reg4, (unsigned int)MAX_STRING_SIZE);
                b->object = fileSystem->Open(b->pathname);
                b->type = FileDescriptor;

                if (b->object == NULL || ((OpenFile*)b->object)->type() != FileHeader::Directory){
                    DEBUG('c', "Unable to open directory %s\n", b->pathname);
                    delete b;
                    machine->WriteRegister(2, 0);                    
                    break;
                } else {
                    int fd = currentThread->space->store_fd(b);
                    if (fd)
                        machine->WriteRegister(2, fd);    
                    else {
                        fileSystem->Close((OpenFile*)b->object);
                        delete [] b->pathname;
                        delete b;
                    }
                }    
                
                break;
            }
            
            case SC_Read: {
                DEBUG('c', "Read syscall on %p, initiated by user program.\n", reg6);
                
                machine->WriteRegister(2, 0);
                fd_bundle_t* b = currentThread->space->get_fd(reg6);
                if (b){
                    char* buffer = new char[reg5];
                    returnvalue = -1;
                                     
                    if (b->type == FileDescriptor){
                        returnvalue = ((OpenFile*)b->object)->Read(buffer, reg5);
                        DEBUG('c', "Reading file: result %d.\n", returnvalue);
                    }
                    else if (b->type == SocketDescriptor && ((Connection*)b->object)->status() == Connection::ESTABLISHED){
                        DEBUG('c', "Reading socket: receiving %d.\n", reg5);
                        returnvalue = ((Connection*)b->object)->Receive(buffer, reg5);
                        DEBUG('c', "Reading socket: result %d.\n", returnvalue);
                    } 
                    
                    machine->WriteRegister(2, returnvalue);                        

                    copyStringToMachine(buffer, reg4, reg5);
                    delete [] buffer;
                } 
                    
                break;
            }
            
            case SC_ReadDir: {
                DEBUG('c', "List syscall on %p, initiated by user program.\n", reg6);
                
                machine->WriteMem(reg4, 1, 0);
                machine->WriteRegister(2, 0);
                                
                fd_bundle_t* b = currentThread->space->get_fd(reg6);
                if (b && ((OpenFile*)b->object)->type() == (int)FileHeader::Directory){
                    int index = ((OpenFile*)b->object)->Tell();
                    DEBUG('c', "Current index is %d.\n", index);
                    Directory* dir = new Directory;
                    dir->FetchFrom((OpenFile*)b->object);
                    
                    if (dir->count() > index) {
                        copyStringToMachine(dir->get_name(index), reg4, MIN(MAX_STRING_SIZE, reg5));
                        ((OpenFile*)b->object)->Seek(++index);
                        machine->WriteRegister(2, 1);
                    }
                    delete dir;
                }
                break;
            }
            
            case SC_Write: {
                DEBUG('c', "Write syscall on %p, initiated by user program.\n", reg4);
                
                machine->WriteRegister(2, 0);
                    
                fd_bundle_t* b = currentThread->space->get_fd(reg6);
                if (b){   
                    char* buffer = copyStringFromMachine(reg4, reg5);
                                     
                    if (b->type == FileDescriptor){
                        returnvalue = ((OpenFile*)b->object)->Write(buffer, reg5);
                        DEBUG('c', "Reading file: result %d.\n", returnvalue);
                    }
                    else if (b->type == SocketDescriptor && ((Connection*)b->object)->status() == Connection::ESTABLISHED){
                        returnvalue = ((Connection*)b->object)->Send(buffer, reg5);
                        DEBUG('c', "Reading socket: result %d.\n", returnvalue);
                    } else
                        machine->WriteRegister(2, -1);
                        
                    machine->WriteRegister(2, returnvalue);
                    delete [] buffer;
                }
                break;
            }
            
            case SC_Tell: {
                DEBUG('c', "Tell syscall on %p, initiated by user program.\n", reg4);
                fd_bundle_t* b = currentThread->space->get_fd(reg4);
                if (b)
                    machine->WriteRegister(2, ((OpenFile*)b->object)->Tell());
                else
                    machine->WriteRegister(2, -1);
                break;
            }
            
            
            case SC_FileTrunk: {
                DEBUG('c', "Tell syscall on %p, initiated by user program.\n", reg4);
                fd_bundle_t* b = currentThread->space->get_fd(reg4);
                machine->WriteRegister(2, -1);
                if (b){
                    if (((OpenFile*)b->object)->type() == FileHeader::Directory)
                        break;
                    BitMap* bm = fileSystem->bitmapTransaction();
                    ((OpenFile*)b->object)->header()->Allocate(bm, ((OpenFile*)b->object)->Tell() + 1);
                    fileSystem->bitmapCommit(bm);
                    machine->WriteRegister(2, 0);
                }
                break;
            }
            
            case SC_Seek: {
                DEBUG('c', "Seek syscall on %p, initiated by user program.\n", reg4);
                fd_bundle_t* b = currentThread->space->get_fd(reg4);
                if (b && reg5 >= 0){
                    ((OpenFile*)b->object)->Seek(reg5);
                    machine->WriteRegister(2, ((OpenFile*)b->object)->Tell());
                }
                else
                    machine->WriteRegister(2, -1);
                    
                break;
            }
            
            case SC_FileInfo: {
                DEBUG('c', "FileInfo syscall on %p, initiated by user program.\n", reg5);
                
                fd_bundle_t* b = currentThread->space->get_fd(reg5);
                if (b){
                    file_info_t info = {((OpenFile*)b->object)->header()->lastaccess() + BASE_TIME, 
                                        ((OpenFile*)b->object)->header()->permission(),
                                        ((OpenFile*)b->object)->Length(),
                                        ((OpenFile*)b->object)->type()
                    };
                    for (unsigned int i = 0; i < sizeof(file_info_t); i++)
                        machine->WriteMem(reg4 + i, 1, *(((char*)&info) + i));
                    machine->WriteRegister(2, 0);
                }
                else
                    machine->WriteRegister(2, -1);
                    
                break;
            }
            
            case SC_FSInfo: {
                DEBUG('c', "FSInfo syscall on %p, initiated by user program.\n", reg5);                
                
                fs_info_t info = {fileSystem->freeSector(), 
                                  NumSectors,
                                  SectorSize
                };
                for (unsigned int i = 0; i < sizeof(fs_info_t); i++)
                    machine->WriteMem(reg4 + i, 1, *(((char*)&info) + i));
                machine->WriteRegister(2, 0);
                    
                break;
            }
            
            case SC_Close: {
                DEBUG('c', "Close syscall on %p, initiated by user program.\n", reg4);
                
                fd_bundle_t* b = currentThread->space->get_fd(reg4);
                if (b && b->object){
                    if (b->type == FileDescriptor)
                        fileSystem->Close((OpenFile*)b->object);
                    else if (b->type == SocketDescriptor)
                        delete (Connection*)b->object;
                    currentThread->space->del_fd(reg4);
                }
                machine->WriteRegister(2, 0);

                break;
            }

            case SC_Sbrk: {
                DEBUG('c', "Sbrk syscall to move brk of %d pages, initiated by user program.\n", reg4);
                /*! \todo Implementation */
                returnvalue = currentThread->space->Sbrk(reg4);
                machine->WriteRegister(2, returnvalue);
                DEBUG('c', "The previous brk was %d and worth now %d.\n", returnvalue, currentThread->space->Sbrk(0));
                break;
            }
            
            case SC_Move: {
                DEBUG('c', "Move syscall on %p, initiated by user program.\n", reg4);
                
                char* old = copyStringFromMachine(reg4, (unsigned int)MAX_STRING_SIZE);
                char* new_ = copyStringFromMachine(reg5, (unsigned int)MAX_STRING_SIZE);
                
                machine->WriteRegister(2, fileSystem->Move(old, new_));
                
                delete [] old;
                delete [] new_;
                /*! \todo Implementation */
                break;
            }
            
            case SC_Remove: {
                DEBUG('c', "Remove syscall on %p, initiated by user program.\n", reg4);
                
                char* path = copyStringFromMachine(reg4, (unsigned int)MAX_STRING_SIZE);
                
                machine->WriteRegister(2, !fileSystem->Remove(path));
                delete [] path;
                break;
            }
            
            case SC_Changemod: {
                DEBUG('c', "Changemod syscall on fd %p with %d, initiated by user program.\n", reg5, reg4);
                
                machine->WriteRegister(2, -1);
                    
                fd_bundle_t* b = currentThread->space->get_fd(reg5);
                if (b){
                    ((OpenFile*)b->object)->header()->permission(reg4);
                    ((OpenFile*)b->object)->SaveHeader();
                    machine->WriteRegister(2, 0);
                }
                
                break;
            }
            
            case SC_Socket: {
                DEBUG('c', "Socket syscall to %d:%d, local: %d.\n", reg4, reg5, reg6);
                
                machine->WriteRegister(2, -1);
                    
                MailBoxAddress box = 
                    reg6 != -1 ? (postOffice->acquireBox(reg6) ? reg6 : -1): postOffice->assignateBox();
                
                if (box < 0)
                    break;
                    
                fd_bundle_t* b = new fd_bundle_t;
                b->pathname = nullptr;
                b->object = (reg4 == -1 && reg5 == -1) ? new Connection(box) : new Connection(box, reg4, reg5);
                b->type = SocketDescriptor;
                
                if (b->object == nullptr){
                    DEBUG('c', "Unable to open socket %d:%d\n", reg4, reg5);
                    delete b;
                    machine->WriteRegister(2, 0);                    
                    break;
                } else {
                    int fd = currentThread->space->store_fd(b);
                    if (fd)
                        machine->WriteRegister(2, fd);    
                    else {
                        delete (Connection*)b->object;
                        delete b;
                    }
                }    
                
                break;
            }
            
            case SC_Connect: {
                DEBUG('c', "Connect syscall on sd %p with %d tick timeout, initiated by user program.\n", reg5, reg4);
                
                machine->WriteRegister(2, 0);                    
                   
                fd_bundle_t* b = currentThread->space->get_fd(reg5);
                if (b && b->type == SocketDescriptor && ((Connection*)b->object)->status() == Connection::IDLE)
                    machine->WriteRegister(2, ((Connection*)b->object)->Connect(reg4));
                
                break;
            }
            
            case SC_Accept: {
                DEBUG('c', "Accept syscall on sd %p with %d tick timeout, initiated by user program.\n", reg6, reg5);
                
                machine->WriteRegister(2, -1);
                   
                fd_bundle_t* b = currentThread->space->get_fd(reg6);
                if (b && b->type == SocketDescriptor){
                    if (((Connection*)b->object)->Accept(reg5)){
                        remote_peer_struct info = {((Connection*)b->object)->remoteAddr(),
                                                 ((Connection*)b->object)->remotePort()};
                        for (unsigned int i = 0; i < sizeof(remote_peer_struct); i++)
                            machine->WriteMem(reg4 + i, 1, *(((char*)&info) + i));
                    }
                }
                
                break;
            }

            default: {
                printf("Unexpected user mode exception %d %ud\n", which, type);
                ASSERT(FALSE);
            }
        }

        // LB: Do not forget to increment the pc before returning!
        UpdatePC ();
        // End of addition
    } else if (which == BusErrorException){
        DEBUG('E', "SIGBUS on the thread %s. Aborting process\n", currentThread->getName());

        /*! \todo error code */
        do_UserProcessExit(-1);
    } else if (which == PageFaultException){        
        char *buffer = new char[64];
        snprintf(buffer, 64, "SIGSEGV on process %d, thread %d: Address %p (page %d) is invalid.\n", (currentThread->space ? currentThread->space->pid() : 0), currentThread->tid(), (void*)reg4, divRoundDown(reg4, PageSize));
        
        synchconsole->AcquireOutput();
        synchconsole->PutString(buffer);        
        synchconsole->ReleaseOutput();
        
        delete [] buffer;
        DEBUG('E', "SIGSEGV on process %d, thread %d: Address %p (page %d) is invalid.\n", (currentThread->space ? currentThread->space->pid() : 0), currentThread->tid(), (void*)reg4, divRoundDown(reg4, PageSize));

        /*! \todo error code */
        do_UserProcessExit(-1);
    } else if (which == ReadOnlyException){
        char *buffer = new char[64];
        snprintf(buffer, 64, "SIGSEGV on process %d, thread %d: Address %p is readOnly.\n", (currentThread->space ? currentThread->space->pid() : 0), currentThread->tid(), (void*)reg4);
        
        synchconsole->AcquireOutput();
        synchconsole->PutString(buffer);        
        synchconsole->ReleaseOutput();
        
        delete [] buffer;
        
        DEBUG('E', "SIGSEGV on process %d, thread %d: Address %p is readOnly.\n", (currentThread->space ? currentThread->space->pid() : 0), currentThread->tid(), (void*)reg4);

        /*! \todo error code */
        do_UserProcessExit(-1);
    } else if (which == OverflowException){
        DEBUG('E', "SIGABRT Overflowing the stack\n", currentThread->getName());

        /*! \todo error code */
        do_UserProcessExit(-1);
    } else if (which == IllegalInstrException){
        DEBUG('E', "SIGABRT as trying to execute illegal instruction\n", currentThread->getName());

        /*! \todo error code */
        do_UserProcessExit(-1);
    } else if (which == AddressErrorException){
        char *buffer = new char[64];
        snprintf(buffer, 64, "SIGSEGV on process %d, thread %d: Address %p is not aligned on the size requested %d.\n", (currentThread->space ? currentThread->space->pid() : 0), currentThread->tid(), (void*)reg4, reg5);
        
        synchconsole->AcquireOutput();
        synchconsole->PutString(buffer);        
        synchconsole->ReleaseOutput();
        
        delete [] buffer;
        
        DEBUG('E', "SIGABRT after trying to access to a none aligned area to the size requested %d at %p\n", reg5, (void*)reg4);

        /*! \todo error code */
        do_UserProcessExit(-1);
    }

}
