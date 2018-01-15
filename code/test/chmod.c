#include "userlib.h"

int main(int argc, char** argv){
    int argCount;
    //~ OpenFileId file;

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	  argCount = 1;
	  //~ if (!strcmp (*argv, "-l"))
        //~ is_long_listing = 1;
	  //~ else
        //~ path = *argv;
    } 
}
