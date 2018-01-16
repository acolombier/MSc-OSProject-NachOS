// directory.cc 
//    Routines to manage a directory of file names.
//
//    The directory is a table of fixed length entries; each
//    entry represents a single file, and contains the file name,
//    and the location of the file header on disk.  The fixed size
//    of each directory entry means that we have the restriction
//    of a fixed maximum size for file names.
//
//    The constructor initializes an empty directory of a certain size;
//    we use ReadFrom/WriteBack to fetch the contents of the directory
//    from disk, and to write back any modifications back to disk.
//
//    Also, this implementation has the restriction that the size
//    of the directory cannot expand.  In other words, once all the
//    entries in the directory are used, no more files can be created.
//    Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "filesys.h"
#include "directory.h"
#include "system.h"

//----------------------------------------------------------------------
// Directory::Directory
//     Initialize a directory; initially, the directory is completely
//    empty.  If the disk is being formatted, an empty directory
//    is all we need, but otherwise, we need to call FetchFrom in order
//    to initialize it from disk.
//
//    "size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory():
    tableSize(0), parent_sector(0), table(nullptr), mItself(nullptr)  // By default, no parent, and ghost
{
}

//----------------------------------------------------------------------
// Directory::~Directory
//     De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    if (table){
	for (int i = 0; i < tableSize; i++)
	    delete [] table[i].name;
	free(table);
    }
} 

bool
Directory::FetchFrom(OpenFile *file)
{
    ASSERT(file->type() == (int)FileHeader::Directory);
    
    mItself = file;
    
    file->Seek(0);
    if (file->Read((char *)&tableSize, sizeof (int)) != sizeof(int)) return false; 
    
    table = (DirectoryEntry*)malloc(sizeof(DirectoryEntry) * tableSize); // So we can use realloc in a perfect way
    memset(table, 0, sizeof(DirectoryEntry) * tableSize);
    
    if (file->Read((char *)&parent_sector, sizeof (int)) != sizeof(int)) return false; 
    
    for (int i = 0; i < tableSize; i++){
        if (file->Read((char *)&table[i].sector, sizeof(int)) != sizeof(int)) return false; 
        if (file->Read((char *)&table[i].namelen, sizeof(int)) != sizeof(int)) return false; 
        table[i].name = new char[table[i].namelen + 1];
        if (file->Read(table[i].name, table[i].namelen) != table[i].namelen){
            for (int k = i; k >= 0; k--)
                delete [] table[k].name;
            free(table);
            table = nullptr;
            tableSize = 0;
            return false;
        }
        table[i].name[table[i].namelen] = '\0';
    }
    return true;
}


bool
Directory::WriteBack(OpenFile *file)
{
    file->Seek(0);
    if (file->Write((char *)&tableSize, sizeof (int)) != sizeof(int)) return false; 
    if (file->Write((char *) &parent_sector, sizeof (int)) != sizeof(int)) return false;
    
    for (int i = 0; i < tableSize; i++){
        if (file->Write((char *)&table[i].sector, sizeof(int)) != sizeof(int)) return false;
        if (file->Write((char *)&table[i].namelen, sizeof(int)) != sizeof(int)) return false;
        if (file->Write(table[i].name, table[i].namelen) != table[i].namelen) return false;
    }
    return true;
}

//----------------------------------------------------------------------
// Directory::FindIndex
//     Look up file name in directory, and return its location in the table of
//    directory entries.  Return -1 if the name isn't in the directory.
//
//    "name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(const char *name)
{
    for (int i = 0; i < tableSize; i++){
        if (!strcmp(table[i].name, name))
        return i;
    }
    return -1;        // name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
//     Look up file name in directory, and return the disk sector number
//    where the file's header is stored. Return -1 if the name isn't 
//    in the directory.
//
//    "name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(const char *name)
{
    int i = FindIndex(name);

    if (i != -1)
        return table[i].sector;
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
//     Add a file into the directory.  Return TRUE if successful;
//    return FALSE if the file name is already in the directory, or if
//    the directory is completely full, and has no more space for
//    additional file names.
//
//    "name" -- the name of the file being added
//    "newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(const char *name, int newSector, BitMap* freeMap)
{ 
    if (FindIndex(name) == -1){
        BitMap* bm = freeMap ? freeMap : fileSystem->bitmapTransaction();    
        if (!mItself->header()->Allocate(bm, mItself->header()->FileLength() + strlen(name) + DirectoryEntryLen)){
            if (!freeMap) fileSystem->bitmapCommit(bm);
            return false;
        }
        if (!freeMap) fileSystem->bitmapCommit(bm);
        
        table = (DirectoryEntry*)realloc(table, ++tableSize * sizeof(DirectoryEntry));
        table[tableSize - 1].name = new char[strlen(name) + 1];
        table[tableSize - 1].namelen = strlen(name);
        strcpy(table[tableSize - 1].name, name); 
        table[tableSize - 1].sector = newSector;
        return true;
    } else
        return false;
}

//----------------------------------------------------------------------
// Directory::Remove
//     Remove a file name from the directory.  Return TRUE if successful;
//    return FALSE if the file isn't in the directory. 
//
//    "name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(const char *name, BitMap* freeMap)
{ 
    int i = FindIndex(name);

    if (i == -1)
        return FALSE;         // name not in directory        
        
    delete [] table[i].name;
    
    if (i != tableSize - 1){
        table[i].name = table[tableSize - 1].name;
        table[i].namelen = table[tableSize - 1].namelen;
        table[i].sector = table[tableSize - 1].sector;    
    }
    table = (DirectoryEntry*)realloc(table, (--tableSize) * sizeof(DirectoryEntry));
    
    BitMap* bm = freeMap ? freeMap : fileSystem->bitmapTransaction();    
    ASSERT(mItself->header()->Allocate(bm, mItself->header()->FileLength() - strlen(name) - DirectoryEntryLen));
    if (!freeMap) fileSystem->bitmapCommit(bm);
    
    return TRUE;    
}

//----------------------------------------------------------------------
// Directory::List
//     List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
    for (int i = 0; i < tableSize; i++)
        printf("%s\n", table[i].name);
}

//----------------------------------------------------------------------
// Directory::Print
//     List all the file names in the directory, their FileHeader locations,
//    and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new FileHeader;

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++){
        printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
        hdr->FetchFrom(table[i].sector);
        hdr->Print();
    }
    printf("\n");
    hdr->dec_ref();
}

int Directory::count() const {
    return tableSize + 2;
}

OpenFile* Directory::get_item(int i) const {
    ASSERT(i >= 0 && i < count());
    
    switch (i){
    case 0:
        return itself();
    case 1:
        return parent();
    default:
        return new OpenFile(table[i - 2].sector);
    }
    return nullptr;
}

char* Directory::get_name(int i) const {
    ASSERT(i >= 0 && i < count());
    
    switch (i){
    case 0:
        return FileSystem::ITSELF_LABEL;
    case 1:
        return FileSystem::PARENT_LABEL;
    default:
        return table[i - 2].name;
    }
    return nullptr;
}

