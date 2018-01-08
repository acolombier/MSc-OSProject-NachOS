#include "syscall.h"

int main()
{
	int result_code;
	
	PutString("Forking the first process\n");
	Join(ForkExec("./userpages0"), &result_code);
	PutString("\nFirst process finished with code");
	PutInt(result_code);
	PutChar('\n');
	PutString("Forking the second process\n");
	ForkExec("./userpages0");
	PutString("Exiting before it finish\n");
	Yield();
	return 0;
}
