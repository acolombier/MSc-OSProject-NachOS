/*! \file userthread.h */

/*! Kernel part of the UserThreadCreate syscall */
/*!
 * \param f User pointer to the user function to be execute.
 * \param arg User pointer to the args of the function to be execute.
 * \param exit_handler Value used to set the ReturnRegister. Has to be passed and can be computed as it offset could change over the time.
 * \todo Get rid of this last argument and get the ability to compute the return handler
 * \return Thread identifier. If NULL_TID, error when creating thread
 */
extern tid_t do_UserThreadCreate(int f, int arg, int exit_handler);

/*! Kernel part of the UserThreadExit syscall */
extern void do_UserThreadExit(int code);

/*! Kernel part of the UserThreadJoin syscall */
extern int do_UserThreadJoin(tid_t tid, int return_code_ptr);
