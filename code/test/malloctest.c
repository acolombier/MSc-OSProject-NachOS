#include "syscall.h"
#include "mem_alloc.c"

#define NB_MAX_ALLOC 64

int main() {
  char* block_pointer[NB_MAX_ALLOC];
  int count = 0, taille, index;

  memset(block_pointer,0, NB_MAX_ALLOC*sizeof(char*));        

  memory_init();

  while (1) {
    PutString("> ");
    switch (GetChar()) {
    case 'a': {
      GetInt(&taille);
      if (count < NB_MAX_ALLOC){
        if ((block_pointer[count]=memory_alloc(taille)))
          count++;
        else
          PutString("Can't allocate!\n");        
      } else
        PutString("Shell can't alloc more!\n");  
      break;
    }
    case 'f':
      GetInt(&index);
      memory_free(block_pointer[index]);
      break; 
    case 'p':
      index = (int)GetChar(); // to get read of the breakline
      memory_display_state();
      break;
    case EOF:
    case 'q':
      return 0;
      break;
    case 0xa:
      break;
    default:
      PutString("Command not found !\n");
      break;
    }
  }
  return 0;
}
