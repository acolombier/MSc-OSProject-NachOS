#include "frameprovider.h"
#include "system.h"
#include "synch.h"

FrameProvider::FrameProvider (int nitems):
	allocatedPage(nitems), mutex(new Lock("frameProvider"))
{
}

FrameProvider::~FrameProvider(){
	delete mutex;
}

unsigned int FrameProvider::GetEmptyFrame(){
	
	mutex->Acquire();

#ifdef FRAME_ALLOC_SEQUENTIAL
	int pnum = allocatedPage.Find ();
#elif FRAME_ALLOC_RANDOM
	int pnum = -1;

	RandomInit(0);
	while (NumAvailFrame() > 0 && (pnum < 0 || allocatedPage.Test(pnum)))
		pnum = Random() % allocatedPage.bits();
#endif
	
	ASSERT(pnum != -1);
	
	allocatedPage.Mark(pnum);
    DEBUG ('a', "Frame %d is assigned\n", pnum);
	mutex->Release();
	
	memset(machine->mainMemory + (pnum * PageSize), 0, PageSize);
	return (unsigned int)pnum;
}

void FrameProvider::ReleaseFrame(int entry){
	mutex->Acquire();
    DEBUG ('a', "Frame %d released\n", entry);
	allocatedPage.Clear(entry);
	mutex->Release();
}

int FrameProvider::NumAvailFrame(){
	return allocatedPage.NumClear();
}
