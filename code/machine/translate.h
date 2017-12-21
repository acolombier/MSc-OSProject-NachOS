// translate.h 
//	Data structures for managing the translation from 
//	virtual page # -> physical page #, used for managing
//	physical memory on behalf of user programs.
//
//	The data structures in this file are "dual-use" - they
//	serve both as a page table entry, and as an entry in
//	a software-managed translation lookaside buffer (TLB).
//	Either way, each entry is of the form:
//	<virtual page #, physical page #>.
//
// DO NOT CHANGE -- part of the machine emulation -- *Updated to the way we do it in the 21st centuary*
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef TLB_H
#define TLB_H

#include "copyright.h"
#include "utility.h"


// The following class defines an entry in a translation table -- either
// in a page table or a TLB.  Each entry defines a mapping from one 
// virtual page to one physical page.
// In addition, there are some extra bits for access control (valid and 
// read-only) and some bits for usage information (use and dirty).

//

class TranslationEntry {
  public:
  
	enum Type {VALID = 0b1, READ_ONLY = 0b10, USE = 0b100, DIRTY = 0b1000};
			//  start of "mainMemory"
			
	TranslationEntry(unsigned int vpage, unsigned int ppage):
		mFlag(0), virtualPage(vpage), physicalPage(ppage) {}
			
	inline bool valid() const { return mFlag & VALID; }  // If this bit is set, the translation is ignored.
		// (In other words, the entry hasn't been initialized.)
	inline bool readOnly() const { return mFlag & READ_ONLY; } // If this bit is set, the user program is not allowed
		// to modify the contents of the page.
	inline bool use() const { return mFlag & USE; }// This bit is set by the hardware every time the
		// page is referenced or modified.
	inline bool dirty() const { return mFlag & DIRTY; }// This bit is set by the hardware every time the
		// page is modified.
		
	inline void setValid(bool v) {
		mFlag = (v ? mFlag | VALID : mFlag & ~VALID);
	}
	inline void setReadOnly(bool v) {
		mFlag = (v ? mFlag | READ_ONLY : mFlag & ~READ_ONLY);
	}
	inline void setUse(bool v) {
		mFlag = (v ? mFlag | USE : mFlag & ~USE);
	}
	inline void setDirty(bool v) {
		mFlag = (v ? mFlag | DIRTY : mFlag & ~DIRTY);
	}
			
  private:
	char mFlag;  
    unsigned int virtualPage;  	// The page number in virtual memory.
    unsigned int physicalPage; 	// The page number in real memory (relative to the
		
};

#endif
