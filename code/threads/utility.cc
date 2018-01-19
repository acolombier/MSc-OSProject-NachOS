// utility.cc 
//      Debugging routines.  Allows users to control whether to 
//      print DEBUG statements, based on a command line argument.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"

// this seems to be dependent on how the compiler is configured.
// if you have problems with va_start, try both of these alternatives
#if defined(HOST_SNAKE) || defined(HOST_SPARC) || defined(HOST_i386)
#include <stdarg.h>
#else
#include "/usr/include/stdarg.h"
#endif

#define LOWER(c) (IS_UPPER(c) ? c + 32 : c)
#define IS_UPPER(c) (c < 97)

#define KNRM  "\x1B[0m"

// Please, comment next to every const which category correspond
#define KBLK  "\x1B[90m"
#define KRED  "\x1B[91m" // Dynamic alloc
#define KGRN  "\x1B[92m" // Thread
#define KYEL  "\x1B[93m" // Syscall
#define KBLU  "\x1B[94m" // Filesys
#define KMAG  "\x1B[95m" // Disk
#define KCYN  "\x1B[96m" // Network
#define KWHT  "\x1B[97m"

#define KBLK_BLD  "\x1B[1;90m"
#define KRED_BLD  "\x1B[1;91m"
#define KGRN_BLD  "\x1B[1;92m"
#define KYEL_BLD  "\x1B[1;93m"
#define KBLU_BLD  "\x1B[1;94m"
#define KMAG_BLD  "\x1B[1;95m"
#define KCYN_BLD  "\x1B[1;96m"
#define KWHT_BLD  "\x1B[1;97m"

#define EOL   "\033[0m"


static const char *enableFlags = NULL;	// controls which DEBUG messages are printed 

//----------------------------------------------------------------------
// DebugInit
//      Initialize so that only DEBUG messages with a flag in flagList 
//      will be printed.
//
//      If the flag is "+", we enable all DEBUG messages.
//
//      "flagList" is a string of characters for whose DEBUG messages are 
//              to be enabled.
//----------------------------------------------------------------------

void
DebugInit (const char *flagList)
{
    enableFlags = flagList;
}

//----------------------------------------------------------------------
// DebugIsEnabled
//      Return TRUE if DEBUG messages with "flag" are to be printed.
//----------------------------------------------------------------------

bool
DebugIsEnabled (char flag)
{
    if (enableFlags != NULL)
	return (strchr (enableFlags, LOWER(flag)) != 0)
	    || (strchr (enableFlags, '+') != 0);
    else
	return FALSE;
}

//----------------------------------------------------------------------
// DEBUG
//      Print a debug message, if flag is enabled.  Like printf,
//      only with an extra argument on the front.
//----------------------------------------------------------------------

void
DEBUG (char flag, const char *format, ...)
{
    if (DebugIsEnabled (flag)){
	  va_list ap;
	  // You will get an unused variable message here -- ignore it.
	  switch (LOWER(flag)){
		case 'l':
		case 'e':
		  fprintf(stderr, !IS_UPPER(flag) ? KRED : KRED_BLD);
		  break;
		case 't':
		  fprintf(stderr, !IS_UPPER(flag) ? KGRN : KRED_BLD);
		  break;
		case 'c':
		  fprintf(stderr, !IS_UPPER(flag) ? KYEL : KYEL_BLD);
		  break;
		case 'd':
		  fprintf(stderr, !IS_UPPER(flag) ? KMAG : KMAG_BLD);
		  break;
		case 'f':
		  fprintf(stderr, !IS_UPPER(flag) ? KBLU : KBLU_BLD);
		  break;
		case 'n':
		  fprintf(stderr, !IS_UPPER(flag) ? KCYN : KCYN_BLD);
		  break;
	  }
	  va_start (ap, format);
	  vfprintf (stderr, format, ap);
	  va_end (ap);
	  switch (LOWER(flag)){
		case 't':
		case 'l':
		case 'c':
		case 'd':
		case 'f':
		case 'e':
		case 'n':
		  fprintf(stderr, EOL);
		  break;
	  }
	  fflush (stderr);
	}
}
