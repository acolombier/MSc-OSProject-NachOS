// bitmap.c 
//      Routines to manage a bitmap -- an array of bits each of which
//      can be either on or off.  Represented as an array of integers.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "bitmap.h"

//----------------------------------------------------------------------
// BitMap::BitMap
//      Initialize a bitmap with "nitems" bits, so that every bit is clear.
//      it can be added somewhere on a list.
//
//      "nitems" is the number of bits in the bitmap.
//----------------------------------------------------------------------

BitMap::BitMap (int nitems):
    mLock(new Lock("Bitmap Lock"))
{
    numBits = nitems;
    numWords = divRoundUp (numBits, BitsInWord);
    map = new unsigned int[numWords];
    for (int i = 0; i < numBits; i++)
	Clear (i);
}

//----------------------------------------------------------------------
// BitMap::~BitMap
//      De-allocate a bitmap.
//----------------------------------------------------------------------

BitMap::~BitMap ()
{
  // LB: Missing [] in delete directive
  //  delete map;
  delete [] map;
  // End of modification
}

//----------------------------------------------------------------------
// BitMap::Set
//      Set the "nth" bit in a bitmap.
//
//      "which" is the number of the bit to be set.
//----------------------------------------------------------------------

void
BitMap::Mark (int which)
{
    bool single_mode = !mLock->isHeldByCurrentThread();
    
    if (single_mode) mLock->Acquire();
    
    ASSERT (which >= 0 && which < numBits);
    map[which / BitsInWord] |= 1 << (which % BitsInWord);
    
    if (single_mode) mLock->Release();
}

//----------------------------------------------------------------------
// BitMap::Clear
//      Clear the "nth" bit in a bitmap.
//
//      "which" is the number of the bit to be cleared.
//----------------------------------------------------------------------

void
BitMap::Clear (int which)
{
    bool single_mode = !mLock->isHeldByCurrentThread();
    
    if (single_mode) mLock->Acquire();
    
    ASSERT (which >= 0 && which < numBits);
    map[which / BitsInWord] &= ~(1 << (which % BitsInWord));
    
    if (single_mode) mLock->Release();
}

//----------------------------------------------------------------------
// BitMap::Test
//      Return TRUE if the "nth" bit is set.
//
//      "which" is the number of the bit to be tested.
//----------------------------------------------------------------------

bool
BitMap::Test (int which)
{
    bool single_mode = !mLock->isHeldByCurrentThread();
    
    if (single_mode) mLock->Acquire();
    
    ASSERT (which >= 0 && which < numBits);
    

    bool result = map[which / BitsInWord] & (1 << (which % BitsInWord));
    
    if (single_mode) mLock->Release();
    
    return result;
}

//----------------------------------------------------------------------
// BitMap::Find
//      Return the number of the first bit which is clear.
//      As a side effect, set the bit (mark it as in use).
//      (In other words, find and allocate a bit.)
//
//      If no bits are clear, return -1.
//----------------------------------------------------------------------

int
BitMap::Find ()
{
    bool single_mode = !mLock->isHeldByCurrentThread();
    
    if (single_mode) mLock->Acquire();
    
    int i;
    for (i = 0; i < numBits; i++)
	if (!Test (i))
	  {
	      Mark (i);
	      break;
	  }
    
    if (single_mode) mLock->Release();
    
    return i == numBits ? -1 : i;
}

//----------------------------------------------------------------------
// BitMap::NumClear
//      Return the number of clear bits in the bitmap.
//      (In other words, how many bits are unallocated?)
//----------------------------------------------------------------------

int
BitMap::NumClear ()
{
    bool single_mode = !mLock->isHeldByCurrentThread();
    
    if (single_mode) mLock->Acquire();
    
    int count = 0;

    for (int i = 0; i < numBits; i++)
	if (!Test (i))
	    count++;
    
    if (single_mode) mLock->Release();
    
    return count;
}

bool BitMap::unlock() {
    if (!mLock->isHeldByCurrentThread())
	return false;
    mLock->Release();
    return true;
}

//----------------------------------------------------------------------
// BitMap::Print
//      Print the contents of the bitmap, for debugging.
//
//      Could be done in a number of ways, but we just print the #'s of
//      all the bits that are set in the bitmap.
//----------------------------------------------------------------------

void
BitMap::Print ()
{
    bool single_mode = !mLock->isHeldByCurrentThread();
    
    if (single_mode) mLock->Acquire();
    
    printf ("Bitmap set:\n");
    for (int i = 0; i < numBits; i++)
	if (Test (i))
	    printf ("%d, ", i);
    printf ("\n");
    
    if (single_mode) mLock->Release();
}

// These aren't needed until the FILESYS assignment

//----------------------------------------------------------------------
// BitMap::FetchFromFile
//      Initialize the contents of a bitmap from a Nachos file.
//
//      "file" is the place to read the bitmap from
//----------------------------------------------------------------------

void
BitMap::FetchFrom (OpenFile * file)
{
    bool single_mode = !mLock->isHeldByCurrentThread();
    
    if (single_mode) mLock->Acquire();
    
    file->ReadAt ((char *) map, numWords * sizeof (unsigned), 0);
    
    if (single_mode) mLock->Release();
}

//----------------------------------------------------------------------
// BitMap::WriteBack
//      Store the contents of a bitmap to a Nachos file.
//
//      "file" is the place to write the bitmap to
//----------------------------------------------------------------------

void
BitMap::WriteBack (OpenFile * file)
{
    bool single_mode = !mLock->isHeldByCurrentThread();
    
    if (single_mode) mLock->Acquire();
    
    file->WriteAt ((char *) map, numWords * sizeof (unsigned), 0);
    
    if (single_mode) mLock->Release();
}
