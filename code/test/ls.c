#include "userlib.h"
#include "malloc.h"

int main(int argc, char** argv){
    int argCount;
    char is_long_listing = 0;
    char* path = NULL;
    OpenFileId listing_dir;

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	  argCount = 1;
	  if (!strcmp (*argv, "-l"))
        is_long_listing = 1;
	  else
        path = *argv;
    } 
    
    listing_dir = path ? OpenDir(path) : OpenDir("/");
    
    if (!listing_dir){
        PutString("Cannot open the directory \"");PutString(path);PutString("\"\n");
        return -1;
    }
    
    char *buffer = (char*)malloc(512);
    int path_size = path ? strlen(path) : 1;
    
	while (ReadDir(buffer, 512, listing_dir) && *buffer){
        if (is_long_listing){    
            char* fullpath = (char*)malloc(path_size + strlen(buffer) + 2);
            
            OpenFileId curr_file_fd = 0;
            if (strcmp(buffer, ".") == 0)
                curr_file_fd = listing_dir;
            //~ else if (strcmp(buffer, "./") == 0)
                //~ curr_file = ParentDir(listing_dir);
            else {
                strcpy(fullpath, path ? path : "/");
                if (path_size > 1 && path[path_size - 1] != '/'){
                    *(fullpath + path_size) = '/';
                    strcpy(fullpath + path_size + 1, buffer);
                    *(fullpath + path_size + strlen(buffer) + 1) = '\0';
                } else {
                    strcpy(fullpath + path_size, buffer);
                    *(fullpath + path_size + strlen(buffer)) = '\0';
                }
            
                curr_file_fd = Open(fullpath); 
            }   
                
            char perm[] = "d---";            
            char timestamp[25] = "                  ";
            file_info_t file_info = {0, 0, 0, 0};
            
            if (curr_file_fd){
                
                if (FileInfo(&file_info, curr_file_fd)){
                    PutString("Fatal Error\n");
                    return -1;
                }
                
                simple_strftime(timestamp, 25, file_info.date);
                
                perm[0] = file_info.type == FILE_TYPE_DIR ? 'd' : '-';
                perm[1] = file_info.perm & O_R ? 'r' : '-';
                perm[2] = file_info.perm & O_W ? 'w' : '-';
                perm[3] = file_info.perm & O_X ? 'x': '-';    
                
                if (curr_file_fd != listing_dir) Close(curr_file_fd);
            }
            PutString(perm);PutChar('\t');PutString(timestamp);PutChar('\t');PutInt(file_info.size);PutChar('\t');PutString(buffer);
            
            free(fullpath);
        } else 
            PutString(buffer);PutChar('\n');
	}
    
    return 0;
}
