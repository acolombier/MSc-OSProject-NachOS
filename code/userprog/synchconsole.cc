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
	mWriteDone(new Semaphore("write done", 0)),
	mReadingMode(new Semaphore("single/array exclusion", 1))
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
	mReadingMode->P();
	mReadAvail->P();
	int c = Console::GetChar();
	mReadingMode->V();
	return c;	
}

void SynchConsole::PutString(const char s[]){ 
	int i = 0;
	while (i < MAX_STRING_SIZE - 1 && s[i])
		PutChar(s[i++]);
	PutChar(0);	
}

void SynchConsole::GetString(char *s, int n){
	mReadingMode->P();
	
	int i = 0;
	while (i < MIN(MAX_STRING_SIZE, n) - 1){
		mReadAvail->P();
		int c;
		if (!(c = Console::GetChar()) || c == EOF || c == '\n') break;
		s[i++] = c;
	}
	s[i] = 0;	
	mReadingMode->V();
}

