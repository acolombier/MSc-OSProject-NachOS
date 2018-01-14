#include "syscall.h"
#include "userlib.h"
#include "malloc.h"

int main (){
	char buffer[512];
    char **args = NULL;
	int argc, result_code;
	
    while (1){
		PutString("\x1B[32mnach_shell\x1B[31m>\033[0m ");
		GetString(buffer, 512);
        
        argc = 0;        
        args = (char**)malloc(sizeof(char*) * (argc + 1));
        if (!args){
            PutString("Can't allocate memory. Skipping...\n");
            continue;
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
        
		if (strcmp("exit", buffer) == 0)
		break;

		if (strlen(buffer)){
			SpaceId process = ForkExec(buffer, args);
			if (!process){
				PutString("Error! Can't run the command ");
				PutString(buffer);
				PutChar('\n');
			} else {
				PutString("Command ");PutString(buffer);PutString(" called with ");PutInt(argc);PutString(" argument(s) and running with the pid ");PutInt(process);PutChar('\n');
				Join(process, &result_code);
				PutString("Command exit with the code ");
				PutInt(result_code);
				PutChar('\n');
			}
		}
        free(args);
    }
    return 0;
}
