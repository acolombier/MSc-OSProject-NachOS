#include "syscall.h"

int main(int argc, char** argv){  
    if (argc < 3){
        PutString("Usage: ");PutString(*argv);PutString(" <oldpath> <newpath>\n");
        return 1;
    }
    
    char* dest_path = argv[argc - 1];
     
     
    for (argc--, *argv++; argc > 1; argc--, argv ++) {
	  switch (Move(*argv, dest_path)) {
        case E_BLOCK:        
            PutString("Can't move ");PutString(*argv);PutString(" to ");PutString(dest_path);PutString(": directory header can't hold more blocks.\n");
            return E_BLOCK;
            break;
        case E_DIRECTORY:    
            PutString("Can't move ");PutString(*argv);PutString(" to ");PutString(dest_path);PutString(": no blocks free remaining.\n");
            return E_DIRECTORY;
            break;
        case E_DISK:          
            PutString("Can't move ");PutString(*argv);PutString(" to ");PutString(dest_path);PutString(": disk is full.\n");
            return E_DISK;
            break;
        case E_EXIST:              
            PutString("Can't move ");PutString(*argv);PutString(" to ");PutString(dest_path);PutString(": the repository already exist.\n");
            return E_EXIST;
            break;
        case E_NOTDIR:              
            PutString("Can't move ");PutString(*argv);PutString(" to ");PutString(dest_path);PutString(": Parent is not a directory.\n");
            return E_NOTDIR;
            break;
        case E_NOTFOUND:              
            PutString("Can't move ");PutString(*argv);PutString(" to ");PutString(dest_path);PutString(": Parent is not found.\n");
            return E_NOTFOUND;
            break;            
        case E_PERM:              
            PutString("Can't move ");PutString(*argv);PutString(" to ");PutString(dest_path);PutString(": Permission denied.\n");
            return E_PERM;
            break;
        }
    } 
    return 0;
}
