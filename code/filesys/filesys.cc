/*! filesys.cc 
//  Routines to manage the overall operation of the file system.
 *  Implements routines to map from textual file names to files.
 *
 *  Each file in the file system has:
 *     A file header, stored in a sector on disk 
 *      (the size of the file header data structure is arranged
 *      to be precisely the size of 1 disk sector)
 *     A number of data blocks
 *     An entry in the file system directory
 *
 *  The file system consists of several data structures:
 *     A bitmap of free disk sectors (cf. bitmap.h)
 *     A directory of file names and file headers
 *
 *      Both the bitmap and the directory are represented as normal
 *  files.  Their file headers are located in specific sectors
 *  (sector 0 and sector 1), so that the file system can find them 
 *  on bootup.
 *
 *  The file system assumes that the bitmap and directory files are
 *  kept "open" continuously while Nachos is running.
 *
 *  For those operations (such as Create, Remove) that modify the
 *  directory and/or bitmap, if the operation succeeds, the changes
 *  are written immediately back to disk (the two files are kept
 *  open during all this time).  If the operation fails, and we have
 *  modified part of the directory and/or bitmap, we simply discard
 *  the changed version, without writing it back to disk.
 *
 *  Our implementation at this point has the following restrictions:
 *
 *     there is no synchronization for concurrent accesses
 *     files have a fixed size, set when the file is created
 *     files cannot be bigger than about 3KB in size
 *     there is no hierarchical directory structure, and only a limited
 *       number of files can be added to the system
 *     there is no attempt to make the system robust to failures
 *      (if Nachos exits in the middle of an operation that modifies
 *      the file system, it may corrupt the disk)
 */

// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "disk.h"
#include "bitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"
#include "system.h"
#include "synch.h"
#include "addrspace.h"
#include "list.h"

#include <algorithm>

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector       0
#define DirectorySector     1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize     (NumSectors / BitsInByte)
#define DirectoryFileSize   (sizeof (int) + sizeof(DirectoryEntry) * NumDirEntries) //its parents ID + its files

//----------------------------------------------------------------------
// FileSystem::FileSystem
//  Initialize the file system.  If format = TRUE, the disk has
//  nothing on it, and we need to initialize the disk to contain
//  an empty directory, and a bitmap of free sectors (with almost but
//  not all of the sectors marked as free).  
//
//  If format = FALSE, we just have to open the files
//  representing the bitmap and the directory.
//
//  "format" -- should we initialize the disk?
//----------------------------------------------------------------------

char FileSystem::ITSELF_LABEL[2] = ".";
char FileSystem::PARENT_LABEL[3] = "..";
char FileSystem::DELIMITER[2] = "/";

FileSystem::FileSystem(bool format):
	fs_lock(new Lock("FS lock"))
{ 
    DEBUG('F', "Initializing the file system.\n");
    
    ASSERT(strlen(DELIMITER) == 1);
    
    files_table = new openfile_t[MAX_OPEN_FILE];
    memset(files_table, 0, sizeof(openfile_t) * MAX_OPEN_FILE);
    
    if (format) {
        BitMap *freeMap = new BitMap(NumSectors);
        Directory *directory = new Directory;
        FileHeader *mapHdr = new FileHeader;
        FileHeader *dirHdr = new FileHeader(FileHeader::Directory);

        DEBUG('F', "Formatting the file system.\n");

        // First, allocate space for FileHeaders for the directory and bitmap
        // (make sure no one else grabs these!)
        freeMap->Mark(FreeMapSector);       
        freeMap->Mark(DirectorySector);

        // Second, allocate space for the data blocks containing the contents
        // of the directory and bitmap files.  There better be enough space!

        ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
        ASSERT(dirHdr->Allocate(freeMap, 2 * sizeof(int)));

        // Flush the bitmap and directory FileHeaders back to disk
        // We need to do this before we can "Open" the file, since open
        // reads the file header off of disk (and currently the disk has garbage
        // on it!).

        DEBUG('F', "Writing headers back to disk.\n");
        mapHdr->WriteBack(FreeMapSector);    
        dirHdr->WriteBack(DirectorySector);

        // OK to open the bitmap and directory files now
        // The file system operations assume these two files are left open
        // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);

        // Once we have the files "open", we can write the initial version
        // of each file back to disk.  The directory at this point is completely
        // empty; but the bitmap has been changed to reflect the fact that
        // sectors on the disk have been allocated for the file headers and
        // to hold the file data for the directory and bitmap.

        DEBUG('F', "Writing bitmap and directory back to disk.\n");
        freeMap->WriteBack(freeMapFile);     // flush changes to disk
        directory->WriteBack(directoryFile);

        if (DebugIsEnabled('f')) {
            freeMap->Print();
            directory->Print();

            delete freeMap; 
            delete directory; 
            mapHdr->dec_ref(); 
            dirHdr->dec_ref();
        }
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
    }
}

int FileSystem::walkThrough(int* directory_sector, const char* ro_name){    
    OpenFile* directory_descriptor = directoryFile;
    Directory* directory = nullptr;
    int sector = DirectorySector;
    
    char* name = new char[strlen(ro_name) + 1];
    strcpy(name, ro_name);
    
    *directory_sector = 0; // By default, we did not found the directory
    
    const char *element_name = strtok(name, DELIMITER);
	
	while( element_name != NULL ) {			
		if (directory) delete directory;	
		directory = new Directory;
		directory->FetchFrom(directory_descriptor);
		
		if ((sector = directory->Find(element_name)) == -1){
			delete directory;
			if (directory_descriptor != directoryFile) delete directory_descriptor;
			return E_NOTFOUND;
		} else {	
			if (directory_descriptor != directoryFile) delete directory_descriptor;		
			directory_descriptor = new OpenFile(sector);
			if (directory_descriptor->type() != (int)FileHeader::Directory){
				delete directory;
				DEBUG('F', "The parent is of type %p", directory_descriptor->type());
				delete directory_descriptor;
				return E_NOTDIR;
			} 
		}	
		
		element_name = strtok(NULL, DELIMITER);
	}
	*directory_sector = sector;
	
	if (directory) delete directory;
	if (directory_descriptor != directoryFile) delete directory_descriptor;
	
	delete [] name;
	
	return E_SUCESS;
}

//----------------------------------------------------------------------
// FileSystem::Create
//  Create a file in the Nachos file system (similar to UNIX create).
//  Since we can't increase the size of files dynamically, we have
//  to give Create the initial size of the file.
//
//  The steps to create a file are:
//    Make sure the file doesn't already exist
//        Allocate a sector for the file header
//    Allocate space on disk for the data blocks for the file
//    Add the name to the directory
//    Store the new file header on disk 
//    Flush the changes to the bitmap and the directory back to disk
//
//  Return TRUE if everything goes ok, otherwise, return FALSE.
//
//  Create fails if:
//          file is already in directory
//      no free space for file header
//      no free entry for file in directory
//      no free space for data blocks for the file 
//
//  Note that this implementation assumes there is no concurrent access
//  to the file system!
//
//  "name" -- name of file to be created
//  "initialSize" -- size of file to be created
//----------------------------------------------------------------------


int FileSystem::Create(const char *name, int initialSize, FileHeader::Type type)
{   
    initialSize = (type == FileHeader::File ? initialSize : DirectoryFileSize);
    
    fs_lock->Acquire();
    
    int parent_sector, success;

    DEBUG('F', "Creating file %s, size %d\n", name, initialSize);
    
    char *element_name, *parent_name;
    basename(name, &parent_name, &element_name);
    
    
    DEBUG('F', "Basename is %s\n", parent_name);
	if ((success = walkThrough(&parent_sector, parent_name))){
		delete [] element_name;
		delete [] parent_name;
		return success;
	}	
	
	ASSERT(parent_sector != 0);
	
	int sector;
    FileHeader *hdr;	
    Directory *directory;
    OpenFile *parent_element = new OpenFile(parent_sector);
    
	ASSERT(parent_element->type() == (int)FileHeader::Directory);
	
	directory = new Directory;
	directory->FetchFrom(parent_element);
	
    DEBUG('F', "filename is %s\n", element_name);

    if (directory->Find(element_name) != -1)
      success = E_EXIST;          // file is already in directory
    else {  
		BitMap *freeMap = new BitMap(NumSectors);
		freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();   // find a sector to hold the file header
        if (sector == -1)       
            success = E_BLOCK;        // no free block for file header 
        else if (!directory->Add(element_name, sector, freeMap))
            success = E_DIRECTORY;    // no space in directory
        else {
            hdr = new FileHeader(type);
            if (!hdr->Allocate(freeMap, initialSize))
                success = E_DISK;    // no space on disk for data
            else {  
                success = E_SUCESS;
                hdr->WriteBack(sector);       
                if (type == FileHeader::Directory){
					OpenFile* new_file = new OpenFile(sector);
					Directory* new_directory = new Directory;
					new_directory->FetchFrom(new_file);
					new_directory->parent(parent_sector);
					new_directory->WriteBack(new_file);
					delete new_directory;
					delete new_file;
				}
				directory->Add(element_name, sector, freeMap);
                directory->WriteBack(parent_element);
                freeMap->WriteBack(freeMapFile);
            }
			hdr->dec_ref();
        }
        delete freeMap;
    }
    
    DEBUG('F', "filename is %sis now at %d\n", name, sector);
    
    delete directory;
    delete [] parent_name;
	delete parent_element;
	delete [] element_name;
    
    fs_lock->Release();
	
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
//  Open a file for reading and writing.  
//  To open a file:
//    Find the location of the file's header, using the directory 
//    Bring the header into memory
//
//  "name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(const char *name)
{ 
    int sector;
    OpenFile *openFile = NULL;

    char *element_name, *parent_name;
    basename(name, &parent_name, &element_name);
    
	if (walkThrough(&sector, parent_name)){
		delete [] element_name;
		delete [] parent_name;
		return NULL;
	}
    
    fs_lock->Acquire();
    
    int free_block = -1;
    for (int i = 0; i < MAX_OPEN_FILE; i++){
		DEBUG('F', "At table %d: %s <?> %s/%s\n", i, files_table[i].pathname, parent_name, element_name);
		
		if (files_table[i].pathname == nullptr || (files_table[i].file && strncmp(files_table[i].pathname, parent_name, strlen(parent_name)) == 0 && strcmp(files_table[i].pathname + strlen(parent_name) + 2, element_name) == 0)){
			free_block = i;
			break;
		}
	}
	
	if (files_table[free_block].pathname != nullptr){ // if free block actually corresponds to the same file already opened
		files_table[free_block].spaceid->push_back(currentThread->space ? currentThread->space->pid() : 0);
		fs_lock->Release();
		return new OpenFile(files_table[free_block].sector, files_table[free_block].file);
	}
	
	if (free_block < 0){
		fs_lock->Release();
		DEBUG('F', "No more room to open a file\n");
		return nullptr;
	}
		
	openFile = new OpenFile(sector);
	
	ASSERT(openFile->type() == (int)FileHeader::Directory);
	
	if (strlen(element_name)){
		Directory *directory;
		directory = new Directory;
		directory->FetchFrom(openFile);
		
		delete openFile;
		openFile = nullptr;
			
		DEBUG('F', "Opening file %s in %s\n", element_name, parent_name);
		sector = directory->Find(element_name); 
		if (sector >= 0)        
			openFile = new OpenFile(sector);    // name was found in directory 
		else
			DEBUG('F', "File %s not found\n", name);
		delete directory;
	}
	
	files_table[free_block].pathname = new char[strlen(parent_name) + strlen(element_name) + 2];
	strcpy(files_table[free_block].pathname, parent_name);
	*(files_table[free_block].pathname + strlen(parent_name)) = '/';
	strcpy(files_table[free_block].pathname + strlen(parent_name) + 1, element_name);
    delete [] element_name;
	delete [] parent_name;
	files_table[free_block].spaceid = new std::vector<int>;
	files_table[free_block].spaceid->push_back(currentThread->space ? currentThread->space->pid() : 0);
	files_table[free_block].file = openFile->header();
	files_table[free_block].sector = sector;
	
	fs_lock->Release();
    
    return openFile;                // return NULL if not found
}

/*!
 * Close a file and remove it from the table
 *
 *  \param "fd" -- the file descriptor
 */
void FileSystem::Close(OpenFile* openFile){
    fs_lock->Acquire();
    
    int free_block = -1;
    for (int i = 0; i < MAX_OPEN_FILE; i++){
		if (files_table[i].file == openFile->header()){		    
			std::vector<int>::iterator position = std::find(files_table[i].spaceid->begin(), files_table[i].spaceid->end(), currentThread->space ? currentThread->space->pid() : 0);
			ASSERT(position != files_table[i].spaceid->end())
			files_table[i].spaceid->erase(position);
			free_block = i;
			break;
		}
	}
	if (files_table[free_block].spaceid->size() == 0){
		delete [] files_table[free_block].pathname;
		delete files_table[free_block].spaceid;
		delete openFile;
		memset(files_table + free_block, 0, sizeof(openfile_t));
	}
	
	fs_lock->Release();
	
}

/*!
 * FileSystem::Move
 *  Moving a file in the tree.  
 *
 *  \param "oldpath" -- the full path of the file
 *  \param "newpath" -- the new full path  of the file
 *  \return error code
 */
int FileSystem::Move(const char *oldpath, const char *newpath)
{ 
	if (strcmp(oldpath, newpath) == 0)
		return E_SUCESS;
	
    fs_lock->Acquire();
    
    for (int i = 0; i < MAX_OPEN_FILE; i++){
		if (files_table[i].pathname && strcmp(files_table[i].pathname, oldpath) == 0){
			fs_lock->Release();
			DEBUG('F', "File is currently used by %d process and can't be move\n", files_table[i].spaceid->size());
			return E_ISUSED;
		}
	}
		
    int old_parent_sector, new_parent_sector, sector, success;
    OpenFile *openFile_1 = NULL, *openFile_2 = NULL;

    char *prefix_name_old, *suffix_name_old;
    char *prefix_name_new, *suffix_name_new;
    
    basename(oldpath, &prefix_name_old, &suffix_name_old);
    
    DEBUG('F', "Move: looking for the old prefix %s\n", prefix_name_old);
	if ((success = walkThrough(&old_parent_sector, prefix_name_old))){
		delete [] prefix_name_old;
		delete [] suffix_name_old;
		
		fs_lock->Release();
		return success;
	}
    DEBUG('F', "Move: oldpath found in %s at sector %d\n", prefix_name_old, old_parent_sector);
	
    basename(newpath, &prefix_name_new, &suffix_name_new);
	
	if ((success = walkThrough(&new_parent_sector, prefix_name_new))){
		delete [] prefix_name_old;
		delete [] suffix_name_old;
		delete [] prefix_name_new;
		delete [] suffix_name_new;
		
		fs_lock->Release();
		return success;
	}
    DEBUG('F', "Move: newpath found in %s at sector %d\n", prefix_name_new, new_parent_sector);
		
	openFile_1 = new OpenFile(old_parent_sector);
	openFile_2 = new OpenFile(new_parent_sector);
	
	ASSERT(openFile_1->type() == (int)FileHeader::Directory);
	ASSERT(openFile_2->type() == (int)FileHeader::Directory);
	
	Directory *old_directory = new Directory, *new_directory = new Directory;
	old_directory->FetchFrom(openFile_1);
	new_directory->FetchFrom(openFile_2);
	
	delete openFile_1;
	delete openFile_2;
	
	if ((sector = new_directory->Find(suffix_name_new)) != -1){
		openFile_1 = new OpenFile(sector);
		if (openFile_1->type() == (int)FileHeader::Directory){
			new_directory->FetchFrom(openFile_1);
			delete [] suffix_name_new;
			suffix_name_new = suffix_name_old;
			new_parent_sector = sector;
			DEBUG('F', "%s is a directory at %d, moving %s inside\n", newpath, sector, suffix_name_old);
			delete openFile_1;
		} else {
			delete [] prefix_name_old;
			delete [] suffix_name_old;
			delete [] prefix_name_new;
			delete [] suffix_name_new;
			delete old_directory;
			delete new_directory;
			DEBUG('F', "File %s exist\n", newpath);
			
			fs_lock->Release();
			return E_EXIST;
		}			
	}
	
	if ((sector = old_directory->Find(suffix_name_old)) == -1){
		delete [] prefix_name_old;
		delete [] suffix_name_old;
		delete [] prefix_name_new;
		delete [] suffix_name_new;
		delete old_directory;
		delete new_directory;
		DEBUG('F', "File %s not found\n", oldpath);
		fs_lock->Release();
		return E_NOTFOUND;		
	}
	
	DEBUG('F', "Moving file %s in %s\n", oldpath, newpath);
	
	openFile_1 = new OpenFile(old_parent_sector);
	openFile_2 = new OpenFile(new_parent_sector);
	
	DEBUG('F', "Comitting new path\n", oldpath, newpath);
	fs_lock->Release();
	if (!new_directory->Add(suffix_name_new, sector))
		return E_DISK;
	fs_lock->Acquire();
	new_directory->WriteBack(openFile_2);
	openFile_2->header()->WriteBack(new_parent_sector);
	delete new_directory;
	
	DEBUG('F', "Comitting old path\n", oldpath, newpath);
	fs_lock->Release();
	if (!old_directory->Remove(suffix_name_old))
		return E_DISK;
	fs_lock->Acquire();
	old_directory->WriteBack(openFile_1);
	openFile_1->header()->WriteBack(old_parent_sector);
	delete old_directory;
	
	delete openFile_1;
	delete openFile_2;
		
	delete [] prefix_name_old;
	delete [] prefix_name_new;
	if (suffix_name_old != suffix_name_new) delete [] suffix_name_old;
	delete [] suffix_name_new;
	
	
	fs_lock->Release();
	
    return E_SUCESS;
}

//----------------------------------------------------------------------
// FileSystem::Remove
//  Delete a file from the file system.  This requires:
//      Remove it from the directory
//      Delete the space for its header
//      Delete the space for its data blocks
//      Write changes to directory, bitmap back to disk
//
//  Return TRUE if the file was deleted, FALSE if the file wasn't
//  in the file system.
//
//  "name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(const char *name)
{ 
    int parent_sector, sector;
    OpenFile *openFile = NULL;
    
	DEBUG('F', "Deleting file %s\n", name);
    fs_lock->Acquire();
    
    for (int i = 0; i < MAX_OPEN_FILE; i++){
		if (files_table[i].pathname && strcmp(files_table[i].pathname, name) == 0){
			fs_lock->Release();
			DEBUG('F', "File is currently used by %d process and can't be deleted\n", files_table[i].spaceid->size());
			return false;
		}
	}

    char *element_name = nullptr, *parent_name = nullptr;
    basename(name, &parent_name, &element_name);
    
	if (walkThrough(&parent_sector, parent_name)){
		delete [] element_name;
    
		fs_lock->Release();
		DEBUG('F', "Directory %s not found\n", parent_name);
		delete [] parent_name;
		return false;
	}
		
	openFile = new OpenFile(parent_sector);
	
	ASSERT(openFile->type() == (int)FileHeader::Directory);
	
	Directory *directory = new Directory;
	directory->FetchFrom(openFile);
	
	DEBUG('F', "Deleting file %s in %s\n", element_name, parent_name);
	sector = directory->Find(element_name); 
	bool success = false;
	delete [] parent_name;
	
	if (sector != -1){
		FileHeader* fileHdr = new FileHeader;
		fileHdr->FetchFrom(sector);
		
		if (fileHdr->type() == FileHeader::Directory){
			OpenFile* curr_file = new OpenFile(sector);
			Directory* curr_dir = new Directory;
			curr_dir->FetchFrom(curr_file);
			if (curr_dir->count() > 2){	
				delete [] element_name;
				delete directory;
				delete curr_dir;
				delete curr_file;
				DEBUG('F', "Directory %s not empty\n", name);
				fs_lock->Release();
				return false;
			}
			delete curr_dir;
			delete curr_file;
		}

		BitMap* freeMap = new BitMap(NumSectors);
		freeMap->FetchFrom(freeMapFile);
		
		fileHdr->Deallocate(freeMap);       // remove data blocks
		freeMap->Clear(sector);         // remove header block
		directory->Remove(element_name, freeMap);
		freeMap->WriteBack(freeMapFile);        // flush to disk
		directory->WriteBack(openFile);        // flush to disk
		
		fileHdr->dec_ref(); //implicit delete
		delete freeMap;
		success = true;
	}  
	else
		DEBUG('F', "File %s not found\n", name);
		
	delete directory;	
	delete openFile;
	
	fs_lock->Release();
    return success;
} 

void FileSystem::basename(const char* ro_name, char**const  prefix, char**const  suffix) {	    
    char* name = new char[strlen(ro_name) + 1];
    strcpy(name, ro_name);
    
    const char* splitter = strrchr(name, DELIMITER[0]);
    
	unsigned int splitter_pos = splitter - name;
	
	while (splitter_pos == strlen(name) - 1){ //Removing the delimiter sign at the end
		name[strlen(name) - 1] = '\0';
		splitter = strrchr(name, DELIMITER[0]);
		splitter_pos = splitter - name;
	}
	
    if (splitter){
		*prefix = new char[splitter_pos + 1];
		*suffix = new char[strlen(name) - splitter_pos];
		strncpy(*prefix, name, splitter_pos);
		strncpy(*suffix, splitter + 1, strlen(name) - splitter_pos);
		(*prefix)[splitter_pos] = '\0';
		(*suffix)[strlen(name) - splitter_pos - 1] = '\0';
	} else {
		*prefix = new char[2];
		strcpy(*prefix, DELIMITER);
		*suffix = new char[strlen(name) + 1];
		strcpy(*suffix, name);
		(*suffix)[strlen(name)] = '\0';
	}
	
	delete [] name;
}

int FileSystem::freeSector(){
	int sector = 0;
	BitMap* bm = bitmapTransaction();
	sector = bm->NumClear();
	bitmapCommit(bm);
	return sector;
}

//----------------------------------------------------------------------
// FileSystem::List
//  List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List()
{
    Directory *directory = new Directory;

    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}

BitMap* FileSystem::bitmapTransaction(){
	fs_lock->Acquire();
	BitMap* freeMap = new BitMap(NumSectors);
	freeMap->FetchFrom(freeMapFile);
	freeMap->lock();
	return freeMap;
}

bool FileSystem::bitmapCommit(BitMap* freeMap){
	if (!fs_lock->isHeldByCurrentThread())
		return false;
	if (!freeMap->unlock()){
		DEBUG('F', "%s#%d from process %d has provided has commited a wrong transaction. Cancelling its request...\n", currentThread->getName(), currentThread->tid(), currentThread->space->pid());
		fs_lock->Release();
		return false;
	}
	freeMap->WriteBack(freeMapFile);
	fs_lock->Release();
	return true;
}
//----------------------------------------------------------------------
// FileSystem::Print
//  Print everything about the file system:
//    the contents of the bitmap
//    the contents of the directory
//    for each file in the directory,
//        the contents of the file header
//        the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    BitMap *freeMap = new BitMap(NumSectors);
    Directory *directory = new Directory;

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
} 
