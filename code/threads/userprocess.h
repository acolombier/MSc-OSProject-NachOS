/*! \file userprocess.h */

/*! Kernel part of the UserProcessCreate syscall */
/*!
 * \param f User pointer to the user function to be execute.
 * \param arg User pointer to the args of the function to be execute.
 * \param exit_handler Value used to set the ReturnRegister. Has to be passed and can be computed as it offset could change over the time.
 * \todo Get rid of this last argument and get the ability to compute the return handler
 * \return Process identifier. If NULL_TID, error when creating process
 */
extern SpaceId do_UserProcessCreate(OpenFile* executable/*, int argc, char** argv*/, int exit_handler);
extern int do_UserProcessJoin(SpaceId pid, int result_code_ptr);
extern void do_UserProcessExit(int code);
extern void do_UserHalt();
