#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

unsigned int SynchConsole::current_sync = 0;
std::map<unsigned int, SynchConsole*> SynchConsole::Table;

SynchConsole::SynchConsole(char *readFile, char *writeFile):
	Console(readFile, writeFile, &SynchConsole::handlerReadAvail, &SynchConsole::handlerWriteDone, SynchConsole::current_sync),
	mReadAvail(new Semaphore("read avail", 0)),
	mWriteDone(new Semaphore("write done", 0)),
	mSync_pnt(SynchConsole::current_sync++)
{
	SynchConsole::Table.insert ( std::pair<int, SynchConsole*>(mSync_pnt, this) );

}

SynchConsole::~SynchConsole()
{
	SynchConsole::Table.erase (mSync_pnt);
	delete mWriteDone;
	delete mReadAvail;
}

void SynchConsole::PutChar(const char ch){
	Console::PutChar(ch);
	mWriteDone->P();
}

char SynchConsole::GetChar(){
	mReadAvail->P();
	return Console::GetChar();
}

void SynchConsole::PutString(const char s[]){ 
	
}

void SynchConsole::GetString(char *s, int n){
}

void SynchConsole::handlerReadAvail(int sync){
	if (SynchConsole::Table.find(sync) != SynchConsole::Table.end())
		SynchConsole::Table.find(sync)->second->handlerReadAvail();
}
void SynchConsole::handlerWriteDone(int sync){
	if (SynchConsole::Table.find(sync) != SynchConsole::Table.end())
		SynchConsole::Table.find(sync)->second->handlerWriteDone();
}
