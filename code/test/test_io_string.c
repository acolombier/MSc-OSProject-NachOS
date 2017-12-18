#include "syscall.h"
////////////////////////////////////////////////////////////////////////////////
//  testing GetString(char* name, int size) and PutString(char* name)
//  Phase 1 : The input is read from a file that
//            is redirected to the standard input.
//  Phase 2 : The input is read form the keyboard.
// 
//
//
////////////////////////////////////////////////////////////////////////////////

int cmp_strings(char* string1, char* string2, int size){
    int same = 1;
    int index = 0;
    while(same && index < size-1){
	same = (string1[index] == string2[index]);
	index ++;
    }
    return same;
}


/**************************** Phase 1 *****************************************/

/* The provided string has a length that is less than size
 * Input  : "Phase 1 : Correct string\n"
 * Output : If the test passed then PutString("PASSED") and return 1.
 *          If the test failed then PutString("FAILED") and return 0.
 */
int phase1_correct_string(){
    PutString("\tPhase 1 : Correct string  : ");
    int  size = 36;
    char* ref_string = "Phase 1 : This is a correct string";
    char string[size];
    GetString(string, size);
    if(cmp_strings(ref_string,string,size)){
	PutString("PASSED\n");
	return 1;
    }
    else{
	PutString("FAILED\n");
	return 0;
    }
}

/* The provided string is longer than the size of the first 
 * Input   : A string that is very long and can't be read at once
 * Process : The standard input is read twice.
 *           The first call to GetString will read the first part.
 *           The second call to Get String will read the second part.
 * Output : If the test passed then PutString("PASSED") and return 1.
 *          If the test failed then PutString("FAILED") and return 0.
 */
int phase1_too_long_string(){
    PutString("\tPhase 1 : String too long : ");
    char* ref_string = "Phase 1 : This string is longer than what will be read";
    int size1 = 12;
    char string1[size1];
    int size2 = 42;
    char string2[size2];
    // First read
    GetString(string1, size1);
    if(!cmp_strings(ref_string, string1, size1)){
	PutString("FAILED\n");
	return 0;
    }
    // Second read
    GetString(string2, size2);
    if(!cmp_strings(ref_string+size1, string2, size2)){
	PutString("FAILED\n");
	return 0;
    }
    PutString("PASSED\n");
    return 1;
}

/**************************** Phase 2 *****************************************/
void interract() {
    int c;
    while ((c = GetChar()) != EOF)
		PutChar(c);
}

/****************************  Main  ******************************************/
int main() {
    PutString("\nTESTING STRING I/O\n");
	
    //~ interract();
    
    phase1_correct_string();
    //phase1_correct_string();
    phase1_too_long_string();
    Halt();
}
