#include "syscall.h"

void print(char c, int n) {
    int i;
    for (i = 0; i < n; i++) {
        PutChar(c+i);
    }
    PutChar('\n');
}

int main() {
    print('a', 4);
    
    /* Do not call Halt() as SC_Exit is called
     * automatically at the end of the main
     * (as seen in __start in start.S
     */
    //Halt();
}
