#include "syscall.h"
#include "mem_alloc.c"

#define NB_MAX_ALLOC 64
#define NB_MAX_THREAD 16

char thread_running[NB_MAX_THREAD];
  
void* thread_test(void* id){
	while (thread_running[(int)id])
		Yield();
	Yield();
	return NULL;
}

int main() {
  char* block_pointer[NB_MAX_ALLOC];
  int thread_id[NB_MAX_THREAD];
  int count = 0, taille, index;

  memset(block_pointer,0, NB_MAX_ALLOC*sizeof(char*));        
  memset(thread_id,0, NB_MAX_THREAD*sizeof(int));        
  memset(thread_running,0, NB_MAX_THREAD*sizeof(char));        

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
    case 'l': {
		index = (int)GetChar(); // to get read of the breakline
		int t = -1;
		while(thread_running[++t] && t < NB_MAX_THREAD - 1){}
		if (t == NB_MAX_THREAD)
          PutString("Can't run thread!\n");  
		else {
			thread_running[t] = 1;
			thread_id[t] = UserThreadCreate(thread_test, (void*)t);
			if (thread_id[t] == NULL_TID){
				thread_running[t] = 0;
				PutString("Error launching thread!\n");  
			} else {				
				PutString("Thread #");PutInt(t);
				PutString(": launch with id "); 
				PutInt(thread_id[t]);
				PutString("\n"); 
			}
		}
			
      break; 
	}
    case 'j': {
		int t;
		GetInt(&t);
		if (t >= NB_MAX_THREAD || !thread_running[t] || !thread_id[t]){
          PutString("Can't join thread!\n");  			
          break;
		}
		
		PutString("Join thread with id ");PutInt(thread_id[t]);
		PutString("\n");
		thread_running[t] = 0;
		if (UserThreadJoin(thread_id[t], NULL))
			PutString("Thread unjoinable!\n");  			
      break; 
	}
    case 'i': {
		int i;
		GetInt(&i);
		if (!block_pointer[i]){
          PutString("No block found!\n");  			
          break;
		}
    
		PutString("Start of header: ");
		PutInt((unsigned int)block_pointer[i] - BLOCK_SIZE_WITH_PADDING);
		PutString("\nStart of the payload: ");
		PutInt((unsigned int)block_pointer[i]);
		PutString("\nStart of the footer: ");
		PutInt((unsigned int)block_pointer[i] + ((mem_block*)(block_pointer[i] - BLOCK_SIZE_WITH_PADDING))->size);
		PutString("\nEnd of the footer: ");
		PutInt((unsigned int)block_pointer[i] + ((mem_block*)(block_pointer[i] - BLOCK_SIZE_WITH_PADDING))->size + BLOCK_SIZE_WITH_PADDING);
		PutString("\nSize: ");
		PutInt((unsigned int)((mem_block*)(block_pointer[i] - BLOCK_SIZE_WITH_PADDING))->size); 			
		PutString("\n");
      break; 
	}
    case 'p':
      index = (int)GetChar(); // to get read of the breakline
      memory_display_state();
      break;
    case EOF:
    case 'q':
      PutString("Exiting...\n");
      return 0;
      break;
    case 0xa:
      break;
    default:
      PutString("Command not found !\n");
      break;
    }
  }
  PutString("Exiting...\n");
  return 0;
}
