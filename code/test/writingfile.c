#include "syscall.h"

int main()
{
   int num;
   FILE *fptr;
   fptr = fopen("testfile.txt","w");

   if(fptr == NULL)
   {
      PutString("Error!");
      exit(1);
   }

   PutString("Enter num: ");
   num=GetChar();

   PutString(num);
   fclose(fptr);

}
