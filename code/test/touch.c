#include "syscall.h"

int main(int argc, char** argv){
    
    if (argc < 2){
        PutString("Usage: ");PutString(argv[0]);PutString(" <file>\n");
        return 2;
    }
    
    OpenFileId file = Open(argv[1]);
    
    if (!file){
        switch (Create(argv[1], O_RW)) {
        case E_BLOCK:        
            PutString("Can't create ");PutString(argv[1]);PutString(": directory header can't hold more blocks.\n");
            return E_BLOCK;
            break;
        case E_DIRECTORY:    
            PutString("Can't create ");PutString(argv[1]);PutString(": no blocks free remaining.\n");
            return E_DIRECTORY;
            break;
        case E_DISK:          
            PutString("Can't create ");PutString(argv[1]);PutString(": disk is full.\n");
            return E_DISK;
            break;
        case E_EXIST:              
            PutString("Can't create ");PutString(argv[1]);PutString(": the repository already exist.\n");
            return E_EXIST;
            break;
        case E_NOTDIR:              
            PutString("Can't create ");PutString(argv[1]);PutString(": Parent is not a directory.\n");
            return E_NOTDIR;
            break;
        case E_NOTFOUND:              
            PutString("Can't create ");PutString(argv[1]);PutString(": Parent is not found.\n");
            return E_NOTFOUND;
            break;            
        case E_PERM:              
            PutString("Can't create ");PutString(argv[1]);PutString(": Permission denied.\n");
            return E_PERM;
            break;
        }
        file = Open(argv[1]);
        if (!file){
            PutString("Cannot open ");PutString(argv[1]);PutString("\n");
            return 1;
        }
    }
    
    int trash;
    Read((char*)&trash, 4, file);
    
    Close(file);
    
    return 0;
}

