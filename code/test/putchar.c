#include "syscall.h"

void print(char c, int n) {
    int i;
    for (i = 0; i < n; i++) {
        PutChar(c+i);
    }
    PutChar('\n');
}

void interract() {
    int c;
    while ((c = GetChar()) != EOF)
		PutChar(c);
}

int main() {
    print('a', 4);
    //~ interract();
    Halt();
}
