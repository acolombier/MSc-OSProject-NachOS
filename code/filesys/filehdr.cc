// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"
#include "synch.h"

FileHeader::FileHeader(Type type):
        flag((unsigned int)type), numBytes(0), numSectors(0), ref_cnt(1), _lock(new Lock("HeaderFile"))
{
    memset(dataSectors, 0, sizeof(int) * NumDirect);
}

FileHeader::~FileHeader(){
    ASSERT(ref_cnt == 0);
    delete _lock;
}

void FileHeader::dec_ref(){
    _lock->Acquire();
    ref_cnt--;
    if (!ref_cnt){
	delete this;
	return;
    }
    _lock->Release();
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int allocSize)
{ 
    _lock->Acquire();
    int data_to__lloc = allocSize - numBytes, sector_to__lloc = divRoundUp(allocSize, SectorSize) - numSectors; // Data which has to be gathered
    
    int new_first_sector = sector_to__lloc < 0 ? numSectors + sector_to__lloc : numSectors;
    int new_last_sector = sector_to__lloc < 0 ? numSectors : numSectors + sector_to__lloc;
    
    int _last_extended_sector = divRoundDown(new_first_sector, SectorSize), tot_sector__lloc = divRoundUp(data_to__lloc, SectorSize * AllocSector) + divRoundUp(data_to__lloc, SectorSize);
    
    if (freeMap->NumClear() < tot_sector__lloc){
	_lock->Release();
	return false;
    }
	
    DEBUG('f', "Allocating %d data sector.\n", tot_sector__lloc);    	
	
    int* sector = new int[AllocSector];
    if (dataSectors[_last_extended_sector]) // If the last extented sector is 0, it means it not allocated although, it is a valid sector
	synchDisk->ReadSector(dataSectors[_last_extended_sector], (char*)sector);
    else {
	dataSectors[_last_extended_sector] = freeMap->Find();
	DEBUG('f', "Allocate the first extended table sector.\n");  
	memset(sector, 0, SectorSize);
    }
    
    for (int i = new_first_sector; i < new_last_sector; i++){
	if (i / SectorSize != _last_extended_sector){
	    if (sector_to__lloc < 0){ // Free...
		synchDisk->ReadSector(dataSectors[++_last_extended_sector], (char*)sector);
		freeMap->Clear(dataSectors[_last_extended_sector]);
		DEBUG('f', "Free %d a %d-th extended table sector.\n", dataSectors[_last_extended_sector], _last_extended_sector);  
	    } else { // Alloc...
		synchDisk->WriteSector(dataSectors[_last_extended_sector], (char*)sector);
		dataSectors[++_last_extended_sector] = freeMap->Find();
		memset(sector, 0, SectorSize);
		DEBUG('f', "Allocate %d a %d-th extended table sector.\n", dataSectors[_last_extended_sector], _last_extended_sector);  
	    }
	}
	
	if (sector_to__lloc < 0){
	    freeMap->Clear(sector[i % SectorSize]);
	    DEBUG('f', "Free %d a data sector %d in the table sector %d.\n", sector[i % SectorSize], i, _last_extended_sector);
	}
	else {
	    sector[i % SectorSize] = freeMap->Find();	
	    DEBUG('f', "Allocate %d a data sector %d in the table sector %d.\n", sector[i % SectorSize], i, _last_extended_sector);
	}
    }
    synchDisk->WriteSector(dataSectors[_last_extended_sector], (char*)sector);
    delete [] sector;
    
    numBytes = allocSize;
    numSectors  = new_last_sector;
    
    _lock->Release();
    
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    Allocate(freeMap, 0);
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    _lock->Acquire();
    synchDisk->ReadSector(sector, (char *)this);
    _lock->Release();
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    _lock->Acquire();
    synchDisk->WriteSector(sector, (char *)this); 
    _lock->Release();
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{    
    int* sector = new int[AllocSector];    
    synchDisk->ReadSector(dataSectors[(offset / SectorSize) / AllocSector], (char*)sector);
    
    int data_sector = sector[(offset / SectorSize) % SectorSize];
    delete [] sector;
    
    DEBUG('f', "Bytes %d is in the %d sector, in the extented table %d at index %d. its value is %d.\n", 	
			offset, offset / SectorSize, (offset / SectorSize) / AllocSector, (offset / SectorSize) % SectorSize, data_sector);
    
    return data_sector;
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}


//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}
