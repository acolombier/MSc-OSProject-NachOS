#include "syscall.h"
#include "malloc.h"
//~ #include "userlib.h"

int main(int argc, char** argv)
{
	OpenFileId file;
    
	char *buffer = (char*)malloc(512);
    
	if ((file = Open("/user_dir_test.txt"))){
        ChMod(O_W, file);
        Close(file);
	}
    
    Remove("/user_dir_test.txt");
	
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
    
    file_info_t fileinfo;
    FileInfo(&fileinfo, file);
    
    PutString("File size ");PutInt(fileinfo.size);PutString(" and read head is at ");PutInt(Tell(file));PutString("\n");
    Close(file);
    
    
	if (!(file = Open("/mytestfile.txt"))){
		PutString("Can't open the file\n");
		return -1;
	}
	
    int read_size;
	if ((read_size = Read(buffer, 100, file)) >= 0){
		PutString("File content (");PutInt(read_size);PutString(" bytes)\n");
        if (read_size < 100) buffer[read_size] = '\0';
		PutString(buffer);
	} else
		PutString("Nothing to read\n");
        
    FileInfo(&fileinfo, file);
    Seek(file, fileinfo.size - 1);
	
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
