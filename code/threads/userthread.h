/*! \file userthread.h */

/*! Kernel part of the UserThreadCreate syscall */
/*!
 * \param f User pointer to the user function to be execute.
 * \param arg User pointer to the args of the function to be execute.
 */
extern int do_UserThreadCreate(int f, int arg);

/*! Kernel part of the UserThreadExit syscall */
extern void do_UserThreadExit();

extern int do_UserThreadJoin(int tid);
