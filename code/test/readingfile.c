#include "syscall.h"


int main()
{
   int num;
   FILE *fptr;

   if ((fptr = Open("test.txt","r")) == NULL){
       PutString("Error! opening file");

   }

   num=GetChar();

   PutString("Value of n=%d", num);
   Close(fptr);

   return 0;
}
