#include "syscall.h"

int main(int argc, char** argv){
    fs_info_t info;
    
    if (FileSystemInfo(&info))
        return -1;
    
    PutString("Filesystem\t");PutInt(info.block_size / 8);PutString("B-blocks\tUsed\tAvailable\tUse%\tMounted on\n");
    
    PutString("DISK\t\t");PutInt(info.total_block);PutString("\t\t");PutInt(info.total_block - info.free_block);PutString("\t");PutInt(info.free_block);PutString("\t\t");PutInt(((info.total_block - info.free_block) * 100) / info.total_block);PutString("\t/\n");
    
    return 0;
} 

