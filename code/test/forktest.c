#include "syscall.h"

int main()
{
	PutString("Forking the first process\n");
	Join(ForkExec("./userpages0"));
	PutString("\nFirst process finished\n");
	PutString("Forking the second process\n");
	ForkExec("./userpages0");
	PutString("Exiting before it finish\n");
	Yield();
	//~ return 0;
}
