#include "syscall.h"


int main()
{
   int num;
   FILE *fptr;

   if ((fptr = fopen("test.txt","r")) == NULL){
       PutString("Error! opening file");

   }

   num=GetChar();

   PutString("Value of n=%d", num);
   fclose(fptr);

   return 0;
}
