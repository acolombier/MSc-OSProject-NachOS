#include "system.h"
#include "machine.h"

char *copyStringFromMachine(int from, unsigned int max_size) {
    int byte;
    unsigned int i;
    char *buffer = new char[max_size];

    for (i = 0; i < max_size - 1; i++) {
        machine->ReadMem(from + i, 1, &byte);
        if ((char) byte == '\0')
            break;
        buffer[i] = (char) byte;
    }
    buffer[i] = '\0';
    return buffer;
}

void copyStringToMachine(char *string, int to, unsigned max_size) {
    for (unsigned int i = 0; i < max_size - 1; i++) {
        machine->WriteMem(to + i, 1, string[i]);
        if (string[i] == '\0')
            break;
    }
}
