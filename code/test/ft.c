#include "syscall.h"
#include "userlib.h"


int ft_receive();
int ft_send(char *filename);

int main(int argc, char *argv[]) {

    if (argc >= 2) {
        if (!strcmp(argv[1], "-r")) {
            return ft_receive();
        } else if (!strcmp(argv[1], "-s") && argc >= 3) {
            return ft_send(argv[2]);
        }
    }

    PutString("Usage: "); PutString(argv[0]); PutString(" -r\n");
    PutString("       "); PutString(argv[0]); PutString(" -s FILE\n");
    return EXIT_SUCCESS;
}


int ft_receive() {
    PutString("Receiving... (not impl yet)\n");
    return EXIT_SUCCESS;
}

int ft_send(char *filename) {
    OpenFileId file;
    char buffer[513];
    int size;

    file = Open(filename);
    if (file == NULL) {
        PutString("The file <"); PutString(filename); PutString("> does not exist.\n");
        return EXIT_FAILURE;
    }

    do {
        size = Read(buffer, 512, file);
        if (size <= 0) {
            PutString("Error "); PutString(filename); PutString(" reading the file <");
            PutString(filename); PutString(">.\n");
            return EXIT_FAILURE;
        }

        buffer[size] = '\0';
        PutString(buffer);
    } while (size == 512);

    return EXIT_SUCCESS;
}
