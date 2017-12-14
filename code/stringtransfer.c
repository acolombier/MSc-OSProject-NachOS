include "systemcall.h"
include "machine.h"


char * copyStringFromMachine(int from, int max_size) {

  int byte;
  unsigned int i;
  char * buffer = new char[max_size];
  for(i = 0; i < max_size-1; i++) {
    machine->ReadMem(from+i,1, &byte);
    if((char)byte=='\0')
      break;
    buffer[i] = (char) byte;
  }
  buffer[i] = '\0';
  return buffer;
}



void copyStringToMachine(char * string, int to, unsigned max_size) {


  char * buffer = (char *)(machine->mainMemory+to);
  unsigned int i;
  for(i = 0; i < max_size-1; i++) {
    buffer[i] = string[i];
    if(string[i]=='\0')
      break;
  }
}
