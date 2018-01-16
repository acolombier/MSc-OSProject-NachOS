/*! \file syscall.h
 *     Nachos system call interface.  These are Nachos kernel operations
 *     that can be invoked from user programs, by trapping to the kernel
 *    via the "syscall" instruction.
 */

/*    This file is included by user programs and by the Nachos kernel.
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation
 * of liability and disclaimer of warranty provisions.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"

/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */
#define SC_Halt        0
#define SC_Exit        1
#define SC_Exec        2
#define SC_Join        3
#define SC_Create    4
#define SC_Open        5
#define SC_OpenDir        33
#define SC_Read        6
#define SC_Write    7
#define SC_Close    8
#define SC_Fork        9
#define SC_Yield    10
#define SC_PutChar    11
#define SC_GetChar    12
#define SC_PutString    13
#define SC_GetString    14
#define SC_PutInt    15
#define SC_GetInt    16
#define SC_CreateUserThread 17
#define SC_ExitUserThread    18
#define SC_JoinUserThread    19
#define SC_SemaInit 20
#define SC_SemaWait    21
#define SC_SemaPost    22
#define SC_ForkExec 23
#define SC_Kill 24

#define SC_Tell 25
#define SC_Seek 26
#define SC_Size 27

#define SC_Move 28
#define SC_Remove 29
#define SC_ReadDir 30
#define SC_Changemod 31

#define SC_Sbrk 32
#define SC_Time 34

#define ConsoleInput    0
#define ConsoleOutput    1

#undef EOF
/*! \def EOF
    The EOF reprensation on 4 bytes.
*/
#define EOF            0xFFFFFFFF
#define NULL_TID    0xFFFFFFFF


/*! \brief Semaphore provided to the user */
//~ typedef void* sem_t;


#ifdef IN_USER_MODE

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 0xFF
#define NULL 0

#define O_NONE 0
/*! \def O_R
    The reading permission in bytes.
*/
#define O_R 0b01
/*! \def O_W
    The writting permission in bytes.
*/
#define O_W 0b10
/*! \def O_X
    The executing permission in bytes.
*/
#define O_X 0b100
#define O_RW (O_R | O_W)
#define O_RX (O_R | O_X)

typedef unsigned int size_t;
typedef int sema_t;

/* when an address space starts up, it has two open files, representing
 * keyboard input and display output (in UNIX terms, stdin and stdout).
 * Read and Write can be used directly on these, without first opening
 * the console device.
 */


// LB: This part is read only on compiling the test/*.c files.
// It is *not* read on compiling test/start.S


/* The system call interface.  These are the operations the Nachos
 * kernel needs to support, to be able to run user programs.
 *
 * Each of these is invoked by a user program by simply calling the
 * procedure; an assembly language stub stuffs the system call code
 * into a register, and traps to the kernel.  The kernel procedures
 * are then invoked in the Nachos kernel, after appropriate error checking,
 * from the system call entry point in exception.cc.
 */

/* Stop Nachos, and print out performance stats */
void Halt () __attribute__((noreturn));


/* Address space control operations: Exit, Exec, and Join */

/*! This user program is done (status = 0 means exited normally).
 */
void Exit (int status) __attribute__((noreturn));

/* A unique identifier for an executing user program (address space) */
typedef int SpaceId;

/* Run the executable, stored in the Nachos file "name", and return the
 * address space identifier
 */
SpaceId Exec (char *name);

/*! Only return once the the user program "id" has finished.
 * \param pointer to store the result code. NULL to ignore it
 * \return boolean saying if the process has been join
 */
int Join (SpaceId id, int* result_code_ptr);


/* File system operations: Create, Open, Read, Write, Close
 * These functions are patterned after UNIX -- files represent
 * both files *and* hardware I/O devices.
 *
 * If this assignment is done before doing the file system assignment,
 * note that the Nachos file system has a stub implementation, which
 * will work for the purposes of testing out these routines.
 */

/* A unique identifier for an open Nachos file. */
typedef int OpenFileId;


/*!
 * \brief Create a file name at path "name"
 * \param name the path
 * \return return an error code, E_SUCCESS (0) if none
 */
int Create (const char *name, int perm);

/*!
 * \brief Create a dir name at path "name"
 * \param name the path
 * \return return an error code, E_SUCCESS (0) if none
 */
int MakeDir (char *name, int perm);

/*!
 *  Open the Nachos file "name", and return an "OpenFileId" that can
 * be used to read and write to the file.
 */
OpenFileId Open (const char *name);

/*!
 *  Open the Nachos dir "name", and return an "OpenFileId" that can
 * be used to read to the dir.
 */
OpenFileId OpenDir (const char *name);

/*!
 * \brief Change the permission of an open item.
 * \param perm the new permission in octal. You can use \ref O_R, \ref O_W or \ref O_X to make it easier.
 * \param id file descriptor
 * \return Return true if modified
 */
int ChMod (int perm, OpenFileId id);

/* Write "size" bytes from "buffer" to the open file. */
int Write (char *buffer, int size, OpenFileId id);

/*!
 * \brief Read "size" bytes from the open file into "buffer".
 * \param buffer buffer address where data is written
 * \param size size of data read and write
 * \param id file descriptor
 * \return Return the number of bytes actually read -- if the open file isn't \
  long enough, or if it is an I/O device, and there aren't enough \
  characters to read, return whatever is available (for I/O devices, \
  you should always wait until you can return at least one character).
 */
int Read (char *buffer, int size, OpenFileId id);

/*!
 * \brief Read at most "size" of next the file name
 * \param buffer buffer address where next file is written
 * \param size size of data read and write
 * \param id file descriptor
 * \return Return the number of bytes actually read -- if the open file isn't \
  long enough, or if it is an I/O device, and there aren't enough \
  characters to read, return whatever is available (for I/O devices, \
  you should always wait until you can return at least one character).
 */
int ReadDir (char *buffer, int size, OpenFileId id);


/*!
 * \brief Move the reading head of the file
 * \param fd the file
 * \param offset the value where the head should be
 * \return Return the value after beeing set. Might be different if there was a range error
 */
int Seek (OpenFileId fd, int offset);

/*!
 * \brief Get the epoch timestamp of the last access to the file
 * \param fd the file
 * \return Return the epoch timestamp in UTC
 */
int Timestamp (OpenFileId fd);

/*!
 * \brief Get the reading head position
 * \param fd the file
 * \return Return the value
 */
int Tell (OpenFileId fd);

/*!
 * \brief Get the file size of a file
 * \param fd the file
 * \return Return the file size
 */
int Size (OpenFileId fd);

/* Close the file, we're done reading and writing to it. */
void Close (OpenFileId id);

/* Yield the CPU to another runnable thread, whether in this address space
 * or not.
 */
void Yield ();

/*!
 * \brief Print a character on the NachOS console.
 * \param ch a character to be printed
 */
void PutChar(char ch);

/*!
 * \brief Read a character from the NachOS console.
 * \return Return the next characrer in the console buffer
 */
char GetChar();

/*!
 * \brief Print a string on the NachOS console.
 * \param *s a string to be printed
 */
void PutString(char *s);

/*!
 * \brief Read a string from the NachOS console.
 * \param *s a string to be read.
 * \param n an integer describing the maximum number of characters to read. This number must be lower than MAX_STRING_SIZE.
 */
void GetString(char *s, int n);

/*!
    \brief Write integer in the STDOUT using ascii representation.
    \param n The integer to put on the NachOS STDOUT.
*/
void PutInt(int n);
/*!
    \brief Read integer in the STDOUT using ascii representation.
    \param *n Pointer to an integer where the read int from ascii STDIN will be stored.
*/
void GetInt(int *n);

/*! \brief Create a user thread
 *  \param f User pointer to the user function to execute. The function signature must be  following this signature void* f(void *)
 *     \param arg User pointer to the args of the function to execute.
 */
int UserThreadCreate(void* f(void *arg), void *arg);

/*! \brief Exit a user thread
 * \param result_code resolut code to return to any waiting thread.
 */
void UserThreadExit(int result_code) __attribute__((noreturn));

/*! Should not be called by the user. Default handler in case of return of a thread function to call exit with a code */
void _user_thread_exit_by_return() __attribute__((noreturn));

/*! \brief Wait for the specified thread to finish
 * \param pointer to store the result code. NULL to ignore it
 * \return booolean saying if there in an error
 */
int UserThreadJoin(int tid, void* result_code_ptr);

/*! \brief Semaphore initialiser */
void sem_init(sema_t* s, int e);

/*! \brief Semaphore post */
void sem_post(sema_t s);

/*! \brief Semaphore wait */
void sem_wait(sema_t s);

/*! \brief send a signal
 * \todo signal system */
void Kill(SpaceId pid, char sig);

/*! \brief Fork and run a process 
 * \param *s path the executable
 * \param **args argument to pass to the process. Must finish by a NULL arg
 * \return the pid of the created process, 0 on error 
 */
int ForkExec(char *s, char** args);

/*! \brief Dynamically allocate n bytes */
void *Sbrk(int size);

#endif // IN_USER_MODE

#endif /* SYSCALL_H */
