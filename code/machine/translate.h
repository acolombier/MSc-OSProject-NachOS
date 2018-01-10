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
			
	TranslationEntry(unsigned int vpage = 0, unsigned int ppage = 0):
		mFlag(0), mVirtualPage(vpage), mPhysicalPage(ppage) {}
			
	inline bool valid() const { return mFlag & VALID; }  // If this bit is set, the translation is ignored.
		// (In other words, the entry hasn't been initialized.)
	inline bool readOnly() const { return mFlag & READ_ONLY; } // If this bit is set, the user program is not allowed
		// to modify the contents of the page.
	inline bool use() const { return mFlag & USE; }// This bit is set by the hardware every time the
		// page is referenced or modified.
	inline bool dirty() const { return mFlag & DIRTY; }// This bit is set by the hardware every time the
		// page is modified.
		
	inline void valid(bool v) {
		mFlag = (v ? mFlag | VALID : mFlag & ~VALID);
	}
	inline void setValid() { valid(true); }
	inline void clearValid() { valid(false); }
	
	inline void readOnly(bool v) {
		mFlag = (v ? mFlag | READ_ONLY : mFlag & ~READ_ONLY);
	}
	inline void setReadOnly() { readOnly(true); }
	inline void clearReadOnly() { readOnly(false); }
	
	inline void use(bool v) {
		mFlag = (v ? mFlag | USE : mFlag & ~USE);
	}
	inline void setUse() { use(true); }
	inline void clearUse() { use(false); }
	
	inline void dirty(bool v) {
		mFlag = (v ? mFlag | DIRTY : mFlag & ~DIRTY);
	}
	inline void setDirty() { dirty(true); }
	inline void clearDirty() { dirty(false); }
	
	inline void virtualPage(unsigned int p) {
		mVirtualPage = p;
	}
	
	inline void physicalPage(unsigned int p) {
		mPhysicalPage = p;
	}
	
	inline unsigned int virtualPage() {
		return mVirtualPage;
	}
	
	inline unsigned int physicalPage() {
		return mPhysicalPage;
	}
			
  protected:
	char mFlag;  
    unsigned int mVirtualPage;  	// The page number in virtual memory.
    unsigned int mPhysicalPage; 	// The page number in real memory (relative to the
		
};

#endif
