#include "syscall.h"


int main() {
    PutString("Input two characters.\n");
    char *c;
    char *d;
    GetString(c,10);
    GetString(d,10);
    PutString(c);
    PutChar('\n');
    PutString(d);
    PutChar('\n');
    Halt();
}
