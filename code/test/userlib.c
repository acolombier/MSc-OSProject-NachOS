#include "userlib.h"
#include "syscall.h"

int strcmp(char* s1, char* s2){
	while (*s1 == *s2 && *s1 && *s2){
		s1++;
		s2++;
	}
	return *s1 < *s2 ? -1 : (*s1 == *s2 ? 0 : 1);
}

int strncmp(char* s1, char* s2, size_t n){
    int s = 1;
    if (n == 0) return 0;
	while (*s1 == *s2 && *s1 && *s2 && s < n){
		s1++;
		s2++;
        s++;
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

int pow(int n, int e){
    if (e == 0) return 1;
    
    int r = n;
    while (--e)
        r *= n;
    
	return r;
}

int atoi(char* s){
	int i = 0, k, d = 0;
    PutString("Size: ");PutString(s);PutInt(strlen(s));PutChar('\n');
	for (k = strlen(s) - 1; k >= 0 && (s[k] - '0') < 10 && (s[k] - '0') >= 0; k--){
		i = i + ((s[k] - '0') * pow(10, d++));
    }

	if (k == 1 && s[k] == '-'){
		i *= -1;
		k--;
	}
    PutInt(k);PutChar('\n');
	return k > 0 ? -1 : i;
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
char * strtok (char * str, char * delimiters ){
    int size = strlen(str), delim = strlen(delimiters);
    for (int i = 0; i < size; i++)
        if (strncmp(str + i, delimiters, delim) == 0)
            return str + i;
    return NULL;
}
