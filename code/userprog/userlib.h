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

//~ typedef struct sema_struct {
	//~ void* internal;
//~ } sema_t;

int strcmp(char* s1, char* s2){
	while (*s1 == *s2 && *s1 && *s2){
		s1++;
		s2++;
	}
	return *s1 < *s2 ? -1 : (*s1 == *s2 ? 0 : 1);
}

int strlen(char* str){
	int i;
	for (i = 0; str[i]; i++){}
	return i;
}

char* strrchr (char* str, int character){
	for (int i = strlen(str) - 1; i >= 0; i--)
		if (str[i] == character)
			return str + i;
	return 0;
}


const char* getlogin(){
	return "guest";
}

const char* getcwd(){
	/*! \todo should be a syscall leter on, and return the current dir when filesystem implemented */
	return "/";
}

const char* gethostname(){
	return "nachos";
}
int atoi(char* s){
	int i = 0, k;
	for (k = strlen(s) - 1; k >= 1 && (s[k] - '0') < 10; k--)
		i = (i << 2) + s[k] - '0';

	if (k == 1 && s[k] == '-'){
		i *= -1;
		k--;
	}
	return k ? -1 : i;
}

void *memcpy(void *dest, const void *src, size_t n){
	char* out_ptr = (char*)dest, *in_ptr = (char*)src;
	for (int i = 0; i < n; i++){
		*(out_ptr + i) = *(in_ptr + i);
	}
	return dest;
}
void *memset(void *s, int c, size_t n){
	char* out_ptr = (char*)s;
	for (int i = 0; i < n; i++)
		*(out_ptr + i) = (c >> ((i % sizeof(int) * 8))) & 0xFF;
	return s;
}

#endif
