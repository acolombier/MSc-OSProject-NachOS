#include "syscall.h"
#include "malloc.c"
//~ #include "userlib.h"

int main(int argc, char** argv)
{
	OpenFileId file, dir;
	
	char *buffer = (char*)malloc(512);
	
	if (!(dir = OpenDir("/"))){
		PutString("Can't open dir\n");
		return -1;
	}
    
	while (ReadDir(buffer, 512, dir)){
		PutString(buffer);
		PutString("\n");
	}
	
	if ((file = Create("/user_dir_test.txt", O_RW))){
		PutString("Can't create the file\n");
		return -1;
	}
    
	if (!(file = Open("/user_dir_test.txt"))){
		PutString("Can't open the created file\n");
		return -1;
	}
    
	int bytes;
	if ((bytes = Write("\nLet's add some in english now", 31, file)) != 31){
		if (bytes >= 0){
			PutString("Can't write everything in the file. Only \n");
			PutInt(bytes);
			PutString(" bytes written\n");
		}
		else {
			PutString("An error occured: ");
			PutInt(bytes);
			PutString("\n");
		}
		return -1;
	}
    
    ChMod(O_NONE, file);
    PutString("File size ");PutInt(Size(file));PutString(" and read head is at ");PutInt(Tell(file));PutString("\n");
    Close(file);
    
    
	if (!(file = Open("/mytestfile.txt"))){
		PutString("Can't open the file\n");
		return -1;
	}
	
	if (Read(buffer, 100, file)){
		PutString("File content (100 bytes)\n");
		PutString(buffer);
	} else
		PutString("Nothing to read\n");
	
	if ((bytes = Write("\nLet's add some in english now", 31, file)) != 31){
		if (bytes >= 0){
			PutString("Can't write everything in the file. Only \n");
			PutInt(bytes);
			PutString(" bytes written\n");
		}
		else {
			PutString("An error occured: ");
			PutInt(bytes);
			PutString("\n");
		}
		return -1;
	}
    free(buffer);
	Close(file);
	return 0;
}
