#ifndef USER_LIB
#define USER_LIB

#define SIGINT 2
#define SIGKILL 9
#define SIGTERM 15
#define SIGCONT 18
#define SIGSTOP 18
#define SIGTSTP 20

#define NULL 0

#define PageSize 128
#define divRoundDown(n,s)  ((n) / (s))
#define divRoundUp(n,s)    (((n) / (s)) + ((((n) % (s)) > 0) ? 1 : 0))

#include "syscall.h"

int strcmp(char* s1, char* s2);
int strlen(char* str);
char* strrchr (char* str, int character);
const char* getlogin();
const char* getcwd();
const char* gethostname();
int atoi(char* s);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char * strtok (char * str, char * delimiters );

#endif
