#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"


void SynchConsole::handlerReadAvail(int sync){
	if (sync)
		((SynchConsole*)sync)->handlerReadAvail();
}
void SynchConsole::handlerWriteDone(int sync){
	if (sync)
		((SynchConsole*)sync)->handlerWriteDone();
}

SynchConsole::SynchConsole(char *readFile, char *writeFile):
	Console(readFile, writeFile, &SynchConsole::handlerReadAvail, &SynchConsole::handlerWriteDone, (int)this),
	mReadAvail(new Semaphore("read avail", 0)),
	mWriteDone(new Semaphore("write done", 0))
{
}

SynchConsole::~SynchConsole()
{
	delete mWriteDone;
	delete mReadAvail;
}

void SynchConsole::PutChar(const char ch){
	Console::PutChar(ch);
	mWriteDone->P();
}

int SynchConsole::GetChar(){
	mReadAvail->P();
	return Console::GetChar();
}

void SynchConsole::PutString(const char s[]){ 
	
}

void SynchConsole::GetString(char *s, int n){
}

