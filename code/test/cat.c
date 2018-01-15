#include "syscall.h"

int main(int argc, char** argv){
    
    if (argc < 1)
        return 0;
    
    OpenFileId file = Open(*argv);
    
    if (!file)
        return -1;
    
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
