#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

SynchConsole::SynchConsole(char *readFile, char *writeFile):
	Console(readFile, writeFile, this->ReadAvail, this->WriteDone, -1), readAvail(new Semaphore("read avail", 0)), writeDone(new Semaphore("write done", 0)){
}

SynchConsole::~SynchConsole()
{
	delete console;
	delete writeDone;
	delete readAvail;
}

void SynchConsole::PutChar(const char ch){
	Console::PutChar(ch);
	WriteDone->P();
}

char SynchConsole::GetChar(){
	readAvail->P();
	return Console::GetChar(ch);
}

void SynchConsole::PutString(const char s[]){
}

void SynchConsole::GetString(char *s, int n){
}
