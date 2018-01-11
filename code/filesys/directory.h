/*! \file directory.h 
 *	Data structures to manage a UNIX-like directory of file names.
 * 
 *      A directory is a table of pairs: <file name, sector #>,
 *	giving the name of each file in the directory, and 
 *	where to find its file header (the data structure describing
 *	where to find the file's data blocks) on disk.
 *
 *      We assume mutual exclusion is provided by the caller.
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation 
 * of liability and disclaimer of warranty provisions.
 */

#include "copyright.h"

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "openfile.h"

#define FileNameMaxLen 		9	// for simplicity, we assume 
					// file names are <= 9 characters long

// The following class defines a "directory entry", representing a file
// in the directory.  Each entry gives the name of the file, and where
// the file's header is to be found on disk.
//
// Internal data structures kept public so that Directory operations can
// access them directly.

#define DirectoryEntryLen 8

class DirectoryEntry {
  public:
    int namelen;	        // Text name size 
    int sector;				// Location on disk to find the 
					//   FileHeader for this file 
    char* name;	// Text name for file, with +1 for 
					// the trailing '\0'
};

// The following class defines a UNIX-like "directory".  Each entry in
// the directory describes a file, and where to find it on disk.
//
// The directory data structure can be stored in memory, or on disk.
// When it is on disk, it is stored as a regular Nachos file.
//
// The constructor initializes a directory structure in memory; the
// FetchFrom/WriteBack operations shuffle the directory information
// from/to disk. 

class Directory {
  public:
    Directory(); 		// Initialize an empty directory
					// with space for "size" files
    ~Directory();			// De-allocate the directory

    void FetchFrom(OpenFile *file);  	// Init directory contents from disk
    void WriteBack(OpenFile *file);	// Write modifications to 
					// directory contents back to disk

    int Find(const char *name);		// Find the sector number of the 
					// FileHeader for file: "name"

    bool Add(const char *name, int newSector, BitMap* freeMap=nullptr);  // Add a file name into the directory

    bool Remove(const char *name, BitMap* freeMap = nullptr);	// Remove a file from the directory

    void List();			// Print the names of all the files
					//  in the directory
    void Print();			// Verbose print of the contents
					//  of the directory -- all the file
					//  names and their contents.
    
    /*!
     * Return the parent (..)
     */
    inline OpenFile* parent() const { return (parent_sector ? new OpenFile(parent_sector) : nullptr); }
    
    /*!
     * Return itself (.)
     */
    inline OpenFile* itself() const { return mItself; }
    
    /*!
     * Set its parent
     * \param sect sector of the parent
     */
    inline void parent(int sect) { this->parent_sector = sect; }
    
    /*!
     * Count the number of items including its own
     * \return number of item
     */
    int count() const;
    
    /*!
     * Get the n-th item
     * \param i n
     * \return n-th item
     */
    OpenFile* get_item(int i) const;
    
    /*!
     * Get the n-th name
     * \param i n
     * \return n-th name
     */
    char* get_name(int i) const;
    
    

  private:
    int tableSize;			// Number of directory entries
    int parent_sector;
    DirectoryEntry *table;		// Table of pairs: 
					// <file name, file header location> 
                    
    OpenFile* mItself;

    int FindIndex(const char *name);	// Find the index into the directory 
					//  table corresponding to "name"
};

#endif // DIRECTORY_H
