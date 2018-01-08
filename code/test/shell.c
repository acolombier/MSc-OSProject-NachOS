#include "syscall.h"
#include "userlib.h"

int main (){
	char cmd[128];
	
    while (1){
		PutString("nach_shell> ");
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
				Join(process);
			}
		}
    }
    return 0;
}
