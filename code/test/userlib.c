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

int strlen(const char* str){
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

char* itoa(int value, char * str, int base){ // Thanks to https://www.geeksforgeeks.org/implement-itoa/
	int i = 0;
    int isNegative = 0;
 
    if (value == 0){
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    if (value < 0 && base == 10)    {
        isNegative = 1;
        value = -value;
    }
 
    while (value != 0){
        int rem = value % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        value = value/base;
    }
 
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0';
 
    reverse(str);
 
    return str;
}

void *memcpy(void *dest, const void *src, size_t n){
	char* out_ptr = (char*)dest, *in_ptr = (char*)src;
	for (int i = 0; i < n; i++){
		*(out_ptr + i) = *(in_ptr + i);
	}
	return dest;
}

void strcpy(char *dest, const char *src){
	char* out_ptr = (char*)dest, *in_ptr = (char*)src;
    int size = strlen(src);
	for (int i = 0; i < size; i++){
		*(out_ptr + i) = *(in_ptr + i);
	}
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

int simple_strftime(char * str, size_t size, int time){
    char buffer[5];
    short day, month = 1, year, hours, mins, secs;
     
    secs = time % 60;
    time -= secs;
    
    mins = (time / 60) % 60;
    time -= mins * 60;
    
    hours = (time / 3600) % 24;
    time -= hours * 3600;
    
    year = (time / (31557600));
    time -= year * 31557600;
    year += 1970;    
    
    day = (time / (3600 * 24));
    time -= day * 3600 * 24;
    day++;
    
    if (time)
        return -1;
    
    short month_day = day;
    while (month_day){
        short nb_days_in_month;
        if ((month % 2) == (month < 9))
            nb_days_in_month = 31;
        else if (month == 2)
            nb_days_in_month = 28 + (year % 4 == 0);
        else
            nb_days_in_month = 30;
        if (month_day < nb_days_in_month)
            break;
        month++;
        month_day -= nb_days_in_month;
    }
    
    size--;
    
    if (size < 3) {*(str) = 0;return 0;}
    strcpy(str, strpad(reverse(itoa(month_day, buffer, 10)), 2, '0'));
    str += 2;
    *(str++) = '/';
    size-=3;
    if (size < 3) {*(str) = 0;return 0;}
    strcpy(str, strpad(reverse(itoa(month, buffer, 10)), 2, '0'));
    str += 2;
    *(str++) = '/';
    size-=3;
    if (size < 5) {*(str) = 0;return 0;}
    strcpy(str, strpad(reverse(itoa(year, buffer, 10)), 4, '0'));
    str += 4;
    *(str++) = ' ';
    size-=5;
    if (size < 3) {*(str) = 0;return 0;}
    strcpy(str, strpad(reverse(itoa(hours, buffer, 10)), 2, '0'));
    str += 2;
    *(str++) = ':';
    size-=3;
    if (size < 3) {*(str) = 0;return 0;}
    strcpy(str, strpad(reverse(itoa(mins, buffer, 10)), 2, '0'));
    str += 2;
    *(str++) = ':';
    size-=3;
    if (size < 2) {*(str) = 0;return 0;}
    strcpy(str, strpad(reverse(itoa(secs, buffer, 10)), 2, '0'));
    *(str) = 0;
    return 1;
}

char* strpad(char* str, size_t i, char padder){
    int str_size = strlen(str);
    char * pointer = str + str_size;
    while (str_size < i){
        *pointer = padder;
        str_size++;
        pointer++;
    }
    *pointer = '\0';
    reverse(str);
    return str;
}

char* reverse(char*str){
    int str_size = strlen(str);
    for (int i = 0; i <divRoundDown(str_size, 2); i++){
        char tmp = str[str_size - i - 1];
        str[str_size - i - 1] = str[i];
        str[i] = tmp;
    }
    return str;
}
