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

/* Start of SYSCALL Code */
#define SC_Halt        0
#define SC_Exit        1
#define SC_Exec        2
#define SC_Join        3
#define SC_Create    4
#define SC_Open        5
#define SC_OpenDir        33
#define SC_ParentDir       36
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
#define SC_FSInfo 27
#define SC_FileTrunk 35

#define SC_Move 28
#define SC_Remove 29
#define SC_ReadDir 30
#define SC_Changemod 31

#define SC_Sbrk 32
#define SC_FileInfo 34

#define SC_Socket 37
#define SC_Connect 38
#define SC_Accept 39

#define ConsoleInput    0
#define ConsoleOutput    1

/* End of SYSCALL Code */

#undef EOF
/*! \def EOF
    The EOF reprensation on 4 bytes.
*/
#define EOF            0xFFFFFFFF
/*! \def NULL_TID
    The NULL_TID reprensents an error. Any function which is supposed to return a thread ID returning is actually throwing an error
*/
#define NULL_TID    0xFFFFFFFF


/* File utils */
/*! \def FILE_TYPE_DIR
    The directory type reprensented as integer.
*/
#define FILE_TYPE_DIR 1

/*! \brief Structure to hold data about a file */
typedef struct file_info_struct {
    int date;
    int perm;
    int size;
    int type;
} file_info_t;

/*! \brief Structure to hold data about a file system */
typedef struct fs_info_struct {
    int free_block;
    int total_block;
    int block_size;
} fs_info_t;

/* Network utils */
/*! \brief Structure to hold data about a file system */
typedef int NetworkAddress;
typedef int MailBoxAddress;

typedef struct remote_peer_struct {
    NetworkAddress addr;
    MailBoxAddress port;
    int attemps; /**< This value holds the number of attemps that have to be done before the conncetion did actually syncronise */
} remote_peer_t;

#ifdef IN_USER_MODE

/* C standard */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 0xFF
#define NULL 0
typedef unsigned int size_t;

/* Permission */
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
#define O_RW 0b11
#define O_RX 0b101

/* Resouces error code */
/*! 
 * The details of those error code are detaild in \ref filesys.h
 * \todo Move error code to \ref system.h
 */
#define E_PERM -8
#define E_ISUSED -7
#define E_NOTDIR -6
#define E_NOTFOUND -5
#define E_BLOCK -4
#define E_DIRECTORY -3
#define E_DISK -2
#define E_EXIST -1
#define E_SUCESS 0

/* Divers utils */
/*! \brief Semaphore provided to the user */
typedef int sema_t;

/* Syscalls definition */

/*!
 * \brief Stop Nachos machine if it is the last process that is alive, and print out performance stats.
 */
void Halt () __attribute__((noreturn));

/*!
 * \brief This user program is done (status = 0 means exited normally). Alias of `return <code>` in the main function.
 */
void Exit (int status) __attribute__((noreturn));


/*!
 * \brief Join a process. Alieas to wait on POSIX.
 * \param id only return once the the user program "id" has finished.
 * \param result_code_ptr pointer to store the result code. NULL to ignore it
 * \return Returns 0 if the process has been joined, otherwise returns -1.
 */
int Join (SpaceId id, int* result_code_ptr);

/*!
 * Unique identifier for the process. 
 */
typedef int OpenFileId; /**< Used type to describ a open file. */
typedef int OpenSocketId; /**< Used type to describ a open file. */
typedef int SpaceId;  /**< Used type to describ a concurrent process on the system. */
typedef int ThreadId;  /**< Used type to describ a concurrent process on the system. */


/*!
 * \brief Create a file name at path "name".
 * \param name the path of the file to be created
 * \param perm new file permission in octal. You can use \ref O_R, \ref O_W, \ref 0_X, \ref O_RW or \ref O_RX to make it easier.
 * \return Returns E_SUCCESS (0) on success and an error number on failure.
 */
int Create (const char *name, int perm);

/*!
 * \brief Create a directory name at path "name"
 * \param name the path of the directory to be created
 * \param perm new directory permission in octal. You can use \ref O_R, \ref O_W or \ref O_RW to make it easier.
 * \return Returns E_SUCCESS (0) on success and an error number on failure.
 */
int MakeDir (char *name, int perm);

/*!
 * \brief Open a Nachos file "name" for reading and writing.
 * \param name the path of the file to be opened
 * \return On success returns an "OpenFileId" that can be used to read and write to the file, returns NULL on failure.
 */
OpenFileId Open (const char *name);

/*!
 * \brief Open Nachos directory "name" for reading.
 * \param name the path of the direct to be opened
 * \return On success returns an "OpenFileId" that can be used to read the directory, returns NULL on failure.
 */
OpenFileId OpenDir (const char *name);

/*!
 * \brief Open the parent directory of a directory.
 * \param dir The child directory you want to find the parent of.
 * \return Return NULL if file has no parent (root directory) or if it is not a directory.
 */
OpenFileId ParentDir (OpenFileId dir);

/*!
 * \brief Change the permissions of an open item.
 * \param perm the new permission in octal. You can use \ref O_R, \ref O_W or \ref O_X to make it easier.
 * \param id file descriptor
 * \return Returns false if no error and modified with success, true on failure.
 */
int ChMod (int perm, OpenFileId id);

/*!
 * \brief Delete a file from the filesystem.
 * \param id TODO file path
 * \return Return TODO false if no error, anything else otherwise
 */
int Remove (char* id);

/*!
 * \brief Move a file from the filesystem.
 * \param old the old file path
 * \param new_ the new file path
 * \return Return false if no error, anything else otherwise
 */
int Move (char* old, char* new_);

/*!
 * \brief Write "size" bytes to the open file from "buffer".
 * \param buffer buffer address where data is written
 * \param size size of data read and write
 * \param id file descriptor
 * \return Return the number of bytes actually written if positive, error code if negative.
 */
int Write (char *buffer, int size, OpenFileId id);

/*!
 * \brief Read "size" bytes from the open file into "buffer".
 * \param buffer buffer address where data is written
 * \param size size of data read and write
 * \param id file descriptor
 * \return Return the number of bytes actually read if positive, error code if negative.
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
 * \brief Get a structure containing the epoch timestamp of the last access (date), the permission in octal (perm), the size(size), and is type (file = 0, dir = 1, ...)
 * \param a pointer to the structure to write
 * \param fd the file
 * \return false is no error, anything else if one occur
 */
int FileInfo (file_info_t*, OpenFileId fd);

/*!
 * \brief Get a structure containing the number of free block (free_block), the used block (used_block), and the block size(block_size) to the main file system
 * \param a pointer to the structure to write
 * \return false is no error, anything else if one occur
 */
int FileSystemInfo (fs_info_t*);

/*!
 * \brief Get the reading head position
 * \param fd the file
 * \return Return the value
 */
int Tell (OpenFileId fd);

/*!
 * \brief Trunk the file to the current head position and deallocate the space already allocated after
 * \param fd the file
 */
void Trunk (OpenFileId fd);

/*!
 * \brief Close the related file if found, and deallocating it from the current address space. The file might still be opened in other address spaces
 * \param fd the opened file
 */
void Close (OpenFileId id);

/*!
 * \brief Yield the CPU to another runnable thread, whether in this address space or not.
 */
void Yield ();

/*!
 * \brief Write a char to the output console. This function blocks until the char has been written.
 * \param ch the char to write
 */
void PutChar(char ch);

/*!
 * \brief Read a char to the input console. This function blocks until the char has been read.
 * \return the char read
 */
char GetChar();

/*!
 * \brief Write a string to the output console. This function blocks until the char has been written.
 * \param *s the string to write. This one has to be well typed, using the end of stream caracter `\0`
 */
void PutString(char *s);

/*!
 * \brief Read a string to the input console. This function blocks until the n char has been read or until either a break line is prompt or a end of file.
 * \param *s the string to write the read char.
 * \param n the maximun number of char. This value can't acceed the \ref MAX_STRING_SIZE value
 * \return the number of char actually read. If < 0, return a code control such as \ref EOF 
 */
int GetString(char *s, int n);

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
 *  \param arg User pointer to the args of the function to execute.
 *  \return the tid of the created thread if success, \ref NULL_TID if an error ocurred.
 */
ThreadId UserThreadCreate(void* f(void *arg), void *arg);

/*! \brief Exit a user thread
 *  \param result_code result code to return to any waiting thread.
 */
void UserThreadExit(void* result_code) __attribute__((noreturn));

/*! Should not be called by the user. Default handler in case of return of a thread function to call exit with a code */
void _user_thread_exit_by_return() __attribute__((noreturn));

/*! \brief Wait for the specified thread to finish
 *  \param pointer to store the result code. NULL to ignore it
 *  \return \ref E_SUCCESS if no error, anything else otherwise
 */
int UserThreadJoin(ThreadId tid, void* result_code_ptr);

/*! \brief Semaphore initialiser. Must be called before any call on this semaphore
 *  \todo implemente a system wrapper to handle a none initialise semaphore, and avoid the system crash
 *  \param * s the pointer to the semapore
 */
void sem_init(sema_t* s, int e);

/*! \brief Semaphore post */
void sem_post(sema_t s);

/*! \brief Semaphore wait */
void sem_wait(sema_t s);

/*! \brief send a signal to a given process
 *  \param pid the process identifier that will receive the signal
 *  \param sig the signal to send
 *  \todo /!\ NOT IMPLEMENTED /!\
 */
void Kill(SpaceId pid, char sig);

/*! \brief Fork and run a process. 
 *  \param *s path to the executable
 *  \param **args arguments to pass to the process. Must finish by a NULL arg. Can be NULL if no arguments.
 *  \return Returns the pid of the created process on success, 0 on error.
 */
int ForkExec(char *s, char** args);

/*! \brief Move the break value of n pages.
 *  \param size the number of pages to allocate if positive, or to free if negative.
 * 	\return If the size is positive the pointer to the first byte now available is returned. If the size is negative the previous value of Sbrk is returned. If the size is negative the pointer
 *  \return Returns the pointer to the first byte now available if size is positive or unavailable if size is negative. If positive size and no more memory available, NULL is returned.
 */
void *Sbrk(int size);

/*! \brief Create a socket object and return its stream descriptor.
 *  \param machineId the address of the remote machine
 *  \param port the port of the remote machine
 *  \return Returns the stream descriptor on success, 0 on error.
 */
OpenSocketId Socket(NetworkAddress machineId, MailBoxAddress port);

/*! \brief Block the socket until a client has procceded to synchronisation with the given socket (by calling \ref Connect).
 *  \param *client the structure to store information about the client
 *  \param timeout the maximun time to wait for (in msec ?). 0 to be none blocking
 *  \param sd the socket descriptor
 *  \return Returns \ref E_SUCCESS if a peerr has syncronised, \ref E_NOTFOUND if timeout
 */
int Accept(remote_peer_t* client, unsigned int timeout, OpenSocketId sd);

/*! \brief Block the socket until a server has procceded to synchronisation with the given socket (by calling \ref Accept)
 *  \param timeout the maximun time to wait for (in msec ?). 0 to be none blocking
 *  \param sd the socket descriptor
 *  \return Returns \ref E_SUCCESS if a peer has syncronised, \ref E_NOTFOUND if timeout.
 */
int Connect(unsigned int timeout, OpenSocketId sd);

#endif // IN_USER_MODE

#endif /* SYSCALL_H */
