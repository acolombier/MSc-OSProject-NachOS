#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "copyright.h"
#include "utility.h"
#include "console.h"
#include "synch.h"
#include "map"

class SynchConsole : public Console {
	public:
		SynchConsole(char *readFile, char *writeFile);
		~SynchConsole();                    // clean up console emulation
		
		void PutChar(const char ch);   // Unix putchar(3S)
		char GetChar();                // Unix getchar(3S)
		
		void PutString(const char *s); // Unix puts(3S)
		void GetString(char *s, int n);       // Unix fgets(3S)	

		void handlerReadAvail() { mReadAvail->V(); }
		void handlerWriteDone() { mWriteDone->V(); }
		
		static void handlerReadAvail(int);
		static void handlerWriteDone(int);
		
		static unsigned int current_sync;
		static std::map<unsigned int, SynchConsole*> Table;
	private:
		Semaphore *mReadAvail;
		Semaphore *mWriteDone;
		unsigned int mSync_pnt;
		
};


#endif // SYNCHCONSOLE_H