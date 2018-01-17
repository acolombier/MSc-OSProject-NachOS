#include "userlib.h"
#include "syscall.h"
#include "malloc.h"

#define SIZE_BUFFER 1024

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
    char *filename = malloc(SIZE_BUFFER), *buffer = malloc(SIZE_BUFFER);
    int size, res;
    OpenFileId file;

    /* TODO: init network connection */
    conn.Accept()
    /* TODO: init network connection */

    /* receiving the filename */
    if (!Receive((char *) &size, sizeof(int))) return EXIT_FAILURE; /* y'a eu une couille */
    if (!Receive(filename, size)) return EXIT_FAILURE; /* y'a eu une couille */

    /* creating/opening the file */
    res = Create(filename, O_RW);
    if (res != E_SUCESS && res != E_EXIST) {  // handle proper overwrite
        PutString("Can't create \"");PutString(filename);PutString("\".\n");
        return EXIT_FAILURE;
    }
    file = Open(filename);
    if (file == NULL) {
        PutString("The file \"");PutString(filename);PutString("\" does not exist.\n");
        return EXIT_FAILURE;
    }

    do {
        /* receiving a part of the file */
        if (!Receive((char *) &size, sizeof(int))) return EXIT_FAILURE; /* y'a eu une couille */
        if (!Receive(buffer, size)) return EXIT_FAILURE; /* y'a eu une couille */

        /* writing that part */
        res = Write(buffer, size, file);
        if (res != size) {
            PutString("Error ");PutInt(size);PutString(" writing the file \"");PutString(filename);PutString("\".\n");
            return EXIT_FAILURE;
        }
    } while (size == SIZE_BUFFER);

    Close(file);

    /*TODO: close network connection */

    free(filename); free(buffer);
    return EXIT_SUCCESS;
}

int ft_send(char *filename) {
    char *buffer = malloc(SIZE_BUFFER);
    int size;
    OpenFileId file;

    /* opening the file */
    file = Open(filename);
    if (file == NULL) {
        PutString("Error opening the file \"");PutString(filename);PutString("\".\n");
        return EXIT_FAILURE;
    }

    /* TODO: init network connection */

    /* sending the filename */
    size = strlen(filename) + 1;
    if (!Send((char *) &size, sizeof(int))) return EXIT_FAILURE; /* y'a eu une couille */
    if (!Send(filename, size)) return EXIT_FAILURE; /* y'a eu une couille */

    do {
        /* reading a part of the file */
        size = Read(buffer, SIZE_BUFFER, file);
        if (size <= 0) {
            PutString("Error ");PutInt(size);PutString(" reading the file \"");PutString(filename);PutString("\".\n");
            return EXIT_FAILURE;
        }

        /* sending that part */
        if (!Send((char *) &size, sizeof(int))) return EXIT_FAILURE; /* y'a eu une couille */
        if (!Send(buffer, size)) return EXIT_FAILURE; /* y'a eu une couille */

    } while (size == SIZE_BUFFER);

    /*TODO: close network connection */

    Close(file);
    free(buffer);
    return EXIT_SUCCESS;
}
