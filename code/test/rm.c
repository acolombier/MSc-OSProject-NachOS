#include "syscall.h"

int main(int argc, char** argv){    
    if (argc < 2){
        PutString("Usage: ");PutString(*argv);PutString(" <directory>\n");
        return -2;
    }
    
    for (argc--, *argv++; argc > 0; argc--, argv ++) {
        if (Remove(*argv)) {
            PutString("Can't remove ");PutString(*argv);PutString(": check that the file exist and that you are allow to delete it.\n");
            return -1;
        }
    } 
    return 0;
}
