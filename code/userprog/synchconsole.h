#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "copyright.h"
#include "utility.h"
#include "console.h"
#include "synch.h"

class SynchConsole : public Console {
	public:
		SynchConsole(char *readFile, char *writeFile);
		~SynchConsole();                    // clean up console emulation
		void PutChar(const char ch);   // Unix putchar(3S)
		char GetChar();                // Unix getchar(3S)
		void PutString(const char *s); // Unix puts(3S)
		void GetString(char *s, int n);       // Unix fgets(3S)	

		void ReadAvail(int) { readAvail->V(); }
		void WriteDone(int) { writeDone->V(); }
	private:
		Semaphore *readAvail;
		Semaphore *writeDone;
};
#endif // SYNCHCONSOLE_H
