#include "syscall.h"
//~ #include "userlib.h"

int main()
{
	OpenFileId file;
	
	char buffer[100];
	
	if (!(file = Open("/mytestfile.txt"))){
		PutString("Can't open the file\n");
		return -1;
	}
	
	if (Read(buffer, 100, file)){
		PutString("File content (100 bytes)\n");
		PutString(buffer);
	} else
		PutString("Nothing to read\n");
	
	int bytes;
	if ((bytes = Write("\nLet's add some in english now", 31, file)) != 31){
		PutString("Can't write everything in the file. Only \n");
		PutInt(bytes);
		PutString(" bytes written\n");
		return -1;
	}
	Close(file);
	return 0;
}
