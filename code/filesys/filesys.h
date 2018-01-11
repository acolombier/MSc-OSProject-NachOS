/*! \file filesys.h 
 *	Data structures to represent the Nachos file system.
 *
 *	A file system is a set of files stored on disk, organized
 *	into directories.  Operations on the file system have to
 *	do with "naming" -- creating, opening, and deleting files,
 *	given a textual file name.  Operations on an individual
 *	"open" file (read, write, close) are to be found in the OpenFile
 *	class (openfile.h).
 *
 *	We define two separate implementations of the file system. 
 *	The "STUB" version just re-defines the Nachos file system 
 *	operations as operations on the native UNIX file system on the machine
 *	running the Nachos simulation.  This is provided in case the
 *	multiprogramming and virtual memory assignments (which make use
 *	of the file system) are done before the file system assignment.
 *
 *	The other version is a "real" file system, built on top of 
 *	a disk simulator.  The disk is simulated using the native UNIX 
 *	file system (in a file named "DISK"). 
 *
 *	In the "real" implementation, there are two key data structures used 
 *	in the file system.  There is a single "root" directory, listing
 *	all of the files in the file system; unlike UNIX, the baseline
 *	system does not provide a hierarchical directory structure.  
 *	In addition, there is a bitmap for allocating
 *	disk sectors.  Both the root directory and the bitmap are themselves
 *	stored as files in the Nachos file system -- this causes an interesting
 *	bootstrap problem when the simulated disk is initialized. 
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation 
 * of liability and disclaimer of warranty provisions.
 */

#ifndef FS_H
#define FS_H

/*! \def E_ISUSE
 * The file is actually in use by an other process and no operation following is allocation can be done
 */
#define E_ISUSED -7

/*! \def E_BLOCK
 * The parent is not a directory
 */
#define E_NOTDIR -6

/*! \def E_BLOCK
 * There is no such file or directory
 */
#define E_NOTFOUND -5

/*! \def E_BLOCK
 * There is no blocks free
 */
#define E_BLOCK -4

/*! \def E_DIRECTORY
 * The directory cannot hold this file as it is full
 */
#define E_DIRECTORY -3

/*! \def E_DISK
 * The dist cannot hold this file as it is full
 */
#define E_DISK -2

/*! \def E_EXIST
 * A file with the same name already exists in this directory
 */
#define E_EXIST -1

/*! \def E_SUCESS
 * No errors happened. This is the so called "error: sucess"
 */
#define E_SUCESS 0

/*! \def NumDirEntries
 * Maximun entries for a directory
 */
#define NumDirEntries       10

/*! \def MAX_OPEN_FILE
 * Maximun file open at the same time
 */
#define MAX_OPEN_FILE 10


#include "copyright.h"
#include "openfile.h"

#ifdef FILESYS_STUB 		// Temporarily implement file system calls as 
				// calls to UNIX, until the real file system
				// implementation is available
class FileSystem {
  public:
    FileSystem(bool format) {}

    bool Create(const char *name, int initialSize) { 
	int fileDescriptor = OpenForWrite(name);

	if (fileDescriptor == -1) return FALSE;
	Close(fileDescriptor); 
	return TRUE; 
	}

    OpenFile* Open(char *name) {
	  int fileDescriptor = OpenForReadWrite(name, FALSE);

	  if (fileDescriptor == -1) return NULL;
	  return new OpenFile(fileDescriptor);
      }

    bool Remove(char *name) { return Unlink(name) == 0; }

};

#else // FILESYS

typedef int SpaceId;

#include "filehdr.h"
#include <vector>

class List;

extern "C" {
	typedef struct openfile_bundle_struct {
		char* pathname;
		FileHeader* file;
		int sector;
		std::vector<int>* spaceid;
	} openfile_t;
}

class Lock;

class FileSystem {
  public:
	static char ITSELF_LABEL[];
	static char PARENT_LABEL[];
	static char DELIMITER[];
	
    FileSystem(bool format);		// Initialize the file system.
					// Must be called *after* "synchDisk" 
					// has been initialized.
    					// If "format", there is nothing on
					// the disk, so initialize the directory
    					// and the bitmap of free blocks.

    int Create(const char *name, int initialSize = 0, FileHeader::Type type = FileHeader::File);  	
					// Create a file (UNIX creat)
					
	int Move(const char *oldpath, const char *newpath);

    OpenFile* Open(const char *name); 	// Open a file (UNIX open)
					
    void Close(OpenFile* );  	
					// Create a file (UNIX creat)

    bool Remove(const char *name); 	// Delete a file (UNIX unlink)
    
    /*!
     * Get the BitMap object ready to work exclusively on it. Must be commited with \ref FileSystem::bitmapCommit after transaction
     * \return the FS bitmap
     */
    BitMap* bitmapTransaction();
    
    /*!
     * Commit the BitMap object. Must be done after calling \ref FileSystem::bitmapTransaction in the same thread
     * \param BitMap* the bitmap object obtained when initialised an transaction
     * \return the FS bitmap
     */
    bool bitmapCommit(BitMap*);    

    void List();			// List all the files in the file system

    void Print();			// List all the files and their contents
    
    /*!
     * Extract The parent path and the filename
     * \param *name The full path
     * \param **prefix the pointer to an unallocated pointer to store parent path. Must be freed after usage.
     * \param **sufffix the pointer to an unallocated pointer to store file name. Must be freed after usage.
     */
    void basename(const char* name, char**const  prefix, char**const  suffix);
    
    /*!
     * Get the free size
     * \return the number of free sector
     */
    int freeSector();

  private:
   OpenFile* freeMapFile;		// Bit map of free disk blocks,
					// represented as a file
   FileHeader* rootHeader;		// "Root" directory -- list of 
					// file names, represented as a file
					
	openfile_t* files_table;
	
	Lock* fs_lock;

	int walkThrough(OpenFile** directory_file, const char* name);
	
	
};

#endif // FILESYS

#endif // FS_H
