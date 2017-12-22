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

class FrameProvider
{
  public:
    FrameProvider (int nitems);	// Initialize a frameprovider // TODO
     ~FrameProvider ();		// De-allocate a frameprovider

	// returns might not be correct
    TranslationEntry GetEmptyFrame();	// retrieve a free frame initialized to 0 by bzero
    TranslationEntry ReleaseFrame(TranslationEntry entry);	// release a frame obtained via GetEmptyFrame
    int NumAvailFrame();	// return the number of available frames

  private:

};
