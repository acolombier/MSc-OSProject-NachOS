#include "syscall.h"
#include "userlib.h"
#include "malloc.h"

int main (){
	
    while (1){
        char buffer[512];
        char **args = NULL;
        int argc, result_code;
        
        memset(buffer, 0, 512);
        
		PutString("\x1B[32mnach_shell\x1B[31m>\033[0m ");
		GetString(buffer, 512);
		
        char forking_mode = 0;
        result_code = 0;
        
        if (buffer[strlen(buffer) - 1] == '&'){
            forking_mode = 1;
            buffer[strlen(buffer) - 1] = '\0';
        }
        
        argc = 1;        
        args = (char**)malloc(sizeof(char*) * (argc + 1));
        if (!args){
            PutString("Can't allocate memory. Skipping...\n");
            continue;
        }
        *args = buffer;
	
	if (strcmp("exit", buffer) == 0)
            break;
	else if (strcmp("\a",buffer)==0){ 
	    PutString("*** ding ***\n");
	    break;
	}
        else if (strcmp("join", buffer) == 0 && argc == 2){
            if (!Join(atoi(args[1]), &result_code)) PutString("Can't join the process\n");
            if (result_code) {PutString("Command exit with the code ");PutInt(result_code);PutChar('\n');}
        }
        
        
        char *arg = strtok(buffer, " ");
        while (arg){
            *arg = '\0';
            if (arg[1] != ' '){
                args = (char**)realloc(args, sizeof(char*) * (argc + 2));
                if (!args){
                    PutString("Can't allocate memory. Exiting...\n");
                    return -1;
                }
                args[argc++] = arg + 1;
            }
            arg = strtok(arg + 1, " ");
        }
        args[argc] = NULL;

		if (strlen(buffer)){
            char* fullpath = NULL;
            if (buffer[0] != '/'){
                fullpath = (char*)malloc(5 + strlen(buffer) + 2);
        
                strcpy(fullpath, "/bin/");
                strcpy(fullpath + 5, buffer);
                fullpath[5 + strlen(buffer)] = '\0';
                OpenFileId is_in_path = Open(fullpath);
                if (is_in_path)
                    Close(is_in_path);
                else {
                    free(fullpath);
                    fullpath = NULL;
                }
            }
            
			SpaceId process = ForkExec(fullpath ? fullpath : buffer, args);
			if (!process){
				PutString("Error! Can't run the command ");
				PutString(buffer);
				PutChar('\n');
			} else {
                free(args);
                if (forking_mode){
                    PutString("Command ");PutString(buffer);PutString(" called with ");PutInt(argc);PutString(" argument(s) and running with the pid ");PutInt(process);PutChar('\n');
                } else {
                    Join(process, &result_code);
                    if (result_code) {PutString("Command exit with the code ");PutInt(result_code);PutChar('\n');}
                }
			}
            if (fullpath)
                free(fullpath);
		}
    }
    return 0;
}
