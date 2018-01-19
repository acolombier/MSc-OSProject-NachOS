#include "syscall.h"
#include "mem_alloc.h"
#include "userlib.h"

#define NB_MAX_ALLOC 64
#define NB_MAX_THREAD 16

char thread_running[NB_MAX_THREAD];

sema_t lock;
  
void* thread_test(void* id){
	while (1){
	    sem_wait(lock);
	    if (!thread_running[(int)id]) break;
	    sem_post(lock);
	    Yield();
	}	
	sem_post(lock);
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
  
  sem_init(&lock, 1);

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
      if (index >= 0 && index < NB_MAX_ALLOC && block_pointer[index]){
        memory_free(block_pointer[index]);
        block_pointer[index] = NULL;
      }
      else PutString("No block to free\n");  
      
      break; 
    case 'l': {
	sem_wait(lock);
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
	sem_post(lock);		
      break; 
    }
    case 'j': {
		sem_wait(lock);
		int t;
		GetInt(&t);
		if (t >= NB_MAX_THREAD || !thread_running[t] || !thread_id[t]){
		    PutString("Can't join thread!\n");  	
		    sem_post(lock);		
		    break;
		}
		
		PutString("Join thread with id ");PutInt(thread_id[t]);
		PutString("\n");
		thread_running[t] = 0;
		int t_to_join = thread_id[t];
		sem_post(lock);
		if (UserThreadJoin(t_to_join, NULL))
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
        index = (int)GetChar(); // to get read of the breakline
      PutString("Exiting...\n");
      sem_wait(lock);
      for (int t = 0; t < NB_MAX_THREAD; t++)
	if (thread_running[t]){		
	    thread_running[t] = 0;
	    int t_to_join = thread_id[t];
	    sem_post(lock);
	    if (UserThreadJoin(t_to_join, NULL))
		PutString("Thread unjoinable!\n"); 
	    sem_wait(lock); 
	}
	sem_post(lock);
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
