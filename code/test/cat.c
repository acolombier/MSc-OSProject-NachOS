#include "syscall.h"

int main(int argc, char** argv){
    
    if (argc < 2){
        PutString("Usage: ");PutString(*argv);PutString(" <file>\n");
        return -2;
    }
    
    OpenFileId file = Open(argv[1]);
    
    if (!file){
        PutString("Cannot open ");PutString(argv[1]);PutString("\n");
        return -1;
    }
    
    int read;  
    
    char buffer[100];
    
	while ((read = Read(buffer, 100, file))){
        if (read < 0){
            switch (read) {
            case E_PERM:              
                PutString("Can't read ");PutString(argv[1]);PutString(": Permission denied.\n");
                return E_PERM;
                break;
            default:              
                PutString("Can't read ");PutString(argv[1]);PutString(": unknown error.\n");
                return E_PERM;
                break;
            }
            break;
        }
        if (read < 100)
            buffer[read] = '\0';
		PutString(buffer);
    }
    
    Close(file);
    
    return 0;
}
