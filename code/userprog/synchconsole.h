#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "copyright.h"
#include "utility.h"
#include "console.h"
#include "synch.h"
#include "map"

//!  A console kernel device adding an other layer over the main console
/*!
 * This synchronisation console offers the possibility to processus to 
 * interrupt themselves in order not to have to handle inside the user 
 * code the availability of the I/O text device. This class is visible by the kernel only.
*/


class SynchConsole : public Console {
	public:    
		//! The class device constructor.
		/*!
		  \param readFile The path to Linux file used is STDIN. Use `null` to use the default STDIN.
		  \param writeFile The path to Linux file used is STDOUT. Use `null` to use the default STDOUT.
		*/
		SynchConsole(char *readFile, char *writeFile);
		~SynchConsole();                    // clean up console emulation
		
		//! Class method to write a single char synchronously.
		/*!
		  \param ch The char to write.
		*/
		void PutChar(const char ch);   // Unix putchar(3S)              // clean up console emulation
		
		//! Class method to get a single char synchronously.
		/*!
		  \return Return a single char. This function might return \ref EOF constant if the end of file has been reached.
		*/
		int GetChar();                // Unix getchar(3S)
		
		//! Class method to write an array of char synchronously.
		/*!
		  \param s The char array to write.
		  
		  The string will be wrote until the char at \ref MAX_STRING_SIZE - 1, or the first \0 char.
		*/
		void PutString(const char *s); // Unix puts(3S)
		
		//! Class method to get an array of char synchronously.
		/*!
		  \param s Pointer the array of char where the read string will be stored. 
		  \param n Maximun number of char that can be read including \0. If this value is greater that \ref MAX_STRING_SIZE, the constant will be used instead. The function will also stop reading if it encounts a \0 char.
		*/
		void GetString(char *s, int n);       // Unix fgets(3S)	

		void handlerReadAvail() { mReadAvail->V(); }
		void handlerWriteDone() { mWriteDone->V(); }
		
		static void handlerReadAvail(int);
		static void handlerWriteDone(int);
		
	private:
		Semaphore *mReadAvail;
		Semaphore *mWriteDone;
		Semaphore *mReadingMode; //TODO: replace by a Lock when tested
		
};


#endif // SYNCHCONSOLE_H
