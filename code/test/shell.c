#include "syscall.h"
#include "userlib.h"

int main (){
	char cmd[128];
	int result_code;
	
    while (1){
		PutString("\x1B[32mnach_shell\x1B[31m>\033[0m ");
		GetString(cmd, 128);

		if (strcmp("exit", cmd) == 0)
		break;

		if (strlen(cmd)){
			SpaceId process = ForkExec(cmd);
			if (!process){
				PutString("Error! Can't run the command ");
				PutString(cmd);
				PutChar('\n');
			} else {
				PutString("Command ");
				PutString(cmd);
				PutString(" running with the pid ");
				PutInt(process);
				PutChar('\n');
				Join(process, &result_code);
				PutString("Command exit with the code ");
				PutInt(process);
				PutChar('\n');
			}
		}
    }
    return 0;
}
