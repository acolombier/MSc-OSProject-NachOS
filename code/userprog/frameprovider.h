/*! \file frameprovider.h
 * 
 * 	Encapsulating allocation of physical pages to virtual pages.
 * 
 * 	This class should also zero-reset the provided pages.
 */

  
 /******************************************************
  *                                                    *
  *                  ~~MERRY X-MAS!! ~~                *
  *                                                    *
  ******************************************************/

#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include "bitmap.h"

class Lock;

class FrameProvider
{
  public:
    FrameProvider (int nitems);	// Initialize a frameprovider // TODO
     ~FrameProvider ();		// De-allocate a frameprovider

    unsigned int GetEmptyFrame();	// retrieve a free frame initialized to 0 by bzero
    void ReleaseFrame(int entry);	// release a frame obtained via GetEmptyFrame
    unsigned int NumAvailFrame();	// return the number of available frames

  private:
	BitMap allocatedPage;
	Lock* mutex;
};
#endif
