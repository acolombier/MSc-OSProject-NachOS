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
		PutString(buffer);
        if (read < 100)
            break;
    }
    
    Close(file);
    
    return 0;
}
