#include "userlib.h"
#include "malloc.h"

int apply_recursive(char* path, OpenFileId file, int octal_perm){
    char *buffer = (char*)malloc(512);

    while (ReadDir(buffer, 512, file) && *buffer){
        if (strcmp(buffer, ".") == 0 || strcmp(buffer, "..") == 0)
            continue;
            
        int path_size = strlen(path);
        char* fullpath = (char*)malloc(path_size + strlen(buffer) + 2);
        
        strcpy(fullpath, path ? path : "/");
        if (path_size > 1 && path[path_size - 1] != '/'){
            *(fullpath + path_size) = '/';
            strcpy(fullpath + path_size + 1, buffer);
            *(fullpath + path_size + strlen(buffer) + 1) = '\0';
        } else {
            strcpy(fullpath + path_size, buffer);
            *(fullpath + path_size + strlen(buffer)) = '\0';
        }
        OpenFileId subfile = Open(path);
        
        if (!subfile){
            PutString("Unknown error\n");
            return -1;
        }
        if (!ChMod(octal_perm, subfile)){
            PutString("Can't set permission to");PutString(path);PutString("\n");
            return -1;
        }
        
        file_info_t subfileinfo;
        FileInfo(&subfileinfo, subfile);
        if (subfileinfo.type == FILE_TYPE_DIR)
            apply_recursive(fullpath, subfile, octal_perm);
        Close(subfile);
        free(path);
    }
    free(buffer);
    return 0;
}

int main(int argc, char** argv){
    int argCount;
    char recursive = 0;
    char octal_perm;
    int extending_mode = 0;
    OpenFileId file;
    
    if (argc < 3){
        PutString("Usage: ");PutString(*argv);PutString(" [-R] [octal|[+|-]string] <file>\n");
        return -2;
    }

    for (argc--, argv++; argc > 1; argc -= argCount, argv += argCount) {
	  argCount = 1;
	  if (!strcmp (*argv, "-R"))
        recursive = 1;
	  else {
          if (strlen(*argv) == 1)
            octal_perm = atoi(*argv);
          else {
              char* str_perm = *argv;
              int perm_size = strlen(str_perm);
              for (int i = 1; i < perm_size; i++){
                  if (str_perm[i] == 'r') octal_perm += O_R;
                  else if (str_perm[i] == 'w') octal_perm += O_W;
                  else if (str_perm[i] == 'x') octal_perm += O_X;
              }
              extending_mode = (str_perm[0] == '-' || str_perm[0] == '+');
              if (str_perm[0] == '-')
                octal_perm = -octal_perm;
          }
      }
    } 
    char* path = *argv;
    file_info_t fileinfo;
    
    file = Open(path);
    
    if (!file){
        PutString("Cannot open ");PutString(path);PutString("\n");
        return -1;
    }
    
    FileInfo(&fileinfo, file);
    
    if (extending_mode)
        octal_perm = fileinfo.perm + octal_perm;
        
    octal_perm = octal_perm > 7 ? 7 : (octal_perm < 0 ? 0 : octal_perm);
    
    if (fileinfo.type == FILE_TYPE_DIR && recursive){
        if (apply_recursive(path, file, octal_perm)){
            PutString("An error has occured\n");
            return -1;
        }
    }
    if (ChMod(octal_perm, file)){
        PutString("Can't set permission\n");
        return -1;
    }
    Close(file);
    return 0;    
}
