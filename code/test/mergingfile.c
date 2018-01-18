#include "syscall.h"

int main()
{
   FILE *fp1 = Open("file1.txt", "r");
   FILE *fp2 = Open("file2.txt", "r");


   FILE *fp3 = Open("file3.txt", "w");
   char c;

   if (fp1 == NULL || fp2 == NULL || fp3 == NULL)
   {
         PutString("Could not open files");
   }

   while ((c = GetChar(fp1)) != EOF)
      PutChar(c, fp3);

   // Copy contents of second file to file3.txt
   while ((c = GetChar(fp2)) != EOF)
      PutChar(c, fp3);

   PutString("Merged file1.txt and file2.txt into file3.txt");

   Close(fp1);
   Cclose(fp2);
   Close(fp3);
   return 0;
}
