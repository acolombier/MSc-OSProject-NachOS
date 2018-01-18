
#include "syscall.h"

#define FILE_NAME "testfile.txt"
int main()
{
  FILE* file_ptr = Open(FILE_NAME, "w");
  Close(file_ptr);
  return 0;
}
