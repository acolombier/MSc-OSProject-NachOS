#include "syscall.h"

int main(int argc, char** argv){    
    if (argc < 2){
        PutString("Usage: ");PutString(*argv);PutString(" <directory>\n");
        return -2;
    }
    
    for (argc--, *argv++; argc > 0; argc--, argv ++) {
	  switch (MakeDir(*argv, O_RW)) {
        case E_BLOCK:        
            PutString("Can't create ");PutString(*argv);PutString(": directory header can't hold more blocks.\n");
            return E_BLOCK;
            break;
        case E_DIRECTORY:    
            PutString("Can't create ");PutString(*argv);PutString(": no blocks free remaining.\n");
            return E_DIRECTORY;
            break;
        case E_DISK:          
            PutString("Can't create ");PutString(*argv);PutString(": disk is full.\n");
            return E_DISK;
            break;
        case E_EXIST:              
            PutString("Can't create ");PutString(*argv);PutString(": the repository already exist.\n");
            return E_EXIST;
            break;
        case E_NOTDIR:              
            PutString("Can't create ");PutString(*argv);PutString(": Parent is not a directory.\n");
            return E_NOTDIR;
            break;
        case E_NOTFOUND:              
            PutString("Can't create ");PutString(*argv);PutString(": Parent is not found.\n");
            return E_NOTFOUND;
            break;            
        case E_PERM:              
            PutString("Can't create ");PutString(*argv);PutString(": Permission denied.\n");
            return E_PERM;
            break;
        }
    } 
    return 0;
}
