#include "syscall.h"

#define NB_PROCESS 12

int main()
{    
    char processes[NB_PROCESS];
    char processname[] = "/userpages0";
    char arg[] = "";
    char *execArgs[] = { processname, arg, NULL};
	
    for (int i = 0; i < NB_PROCESS; i++){
        PutString("Forking process ");PutInt(i);PutString("...\n");
        execArgs[1][0] = '0' + (i % 9);
        processes[i] = ForkExec(processname, execArgs);
        PutString("Forked as ");PutInt(processes[i]);PutString("\n");
    }
	
    for (int i = 0; i < NB_PROCESS; i++){
        int result_code;
        PutString("Joining process ");PutInt(processes[i]);PutString("...\n");
        Join(processes[i], &result_code);
        PutString("Joined process ");PutInt(processes[i]);PutString(" with exit code ");PutInt(result_code);PutString("\n");
    }
	return 0;
}
