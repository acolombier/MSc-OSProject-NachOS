#include "userlib.h"
#include "syscall.h"
#include "malloc.h"

#define SIZE_BUFFER 1024
#define TIMEOUT 1000

int ft_send(char *filename, OpenSocketId sd);
int ft_receive(OpenSocketId sd);


int main(int argc, char *argv[]) {
    int address, port = 2, mode = 0, arg = 1;
    OpenSocketId sd;

    while (mode < 3 && (argc - arg > 1 || (mode != 1 && argc - arg > 0))) {
        if (!strcmp(argv[arg], "-s") || !strcmp(argv[arg], "--send")) {
            if (mode != 0) {
                mode = 4;
                PutString("Incompatible options: can't send and receive at the same time.\n");
            }
            if (arg >= argc - 2) {
                mode = 4;
                PutString("Not enough arguments: missing destination or file.\n");
            }
            arg++;
            address = atoi(argv[arg]);
            arg++;

        } else if (!strcmp(argv[arg], "-r") || !strcmp(argv[arg], "--receive")) {
            if (mode != 0) {
                mode = 4;
                PutString("Incompatible options: can't send and receive at the same time.\n");
            }
            mode = 2;
            arg++;

        } else if (!strcmp(argv[arg], "-p") || !strcmp(argv[arg], "--port")) {
            if (arg >= argc - 1) {
                mode = 4;
                PutString("Not enough arguments: missing port.\n");
            }
            arg++;
            port = atoi(argv[arg]);
            arg++;

        } else if (!strcmp(argv[arg], "-h") || !strcmp(argv[arg], "--help")) {
            mode = 3;
        } else {
            mode = 4;
            PutString("Unknown option \"");PutString(argv[arg]);PutString("\".\n");
        }
    }

    switch (mode) {
        case 1:
            sd = Socket(address, port, -1);
            return ft_send(argv[arg], sd);
            PutString("Sending ");PutString(argv[arg]);
            PutString(" to ");PutInt(address);PutString(" on port ");PutInt(port);PutString(".");
            break;
        case 2:
            sd = Socket(-1, -1, port);
            return ft_receive(sd);
            break;
        case 3:
            PutString("\n       ");PutString(argv[0]);PutString(" - files transfer utility\n\nUSAGE\n       ");
            PutString(argv[0]);PutString(" [OPTION]... FILE\n       ");PutString(argv[0]);
            PutString(" [OPTION]...\n\nOPTIONS\n       -s, --send  ADDRESS\n              send FILE to a files transfer receiver at ADDRESS\n\n       -r, --receive\n              receive file(s) from files transfer sender, does not accept FILE\n              as an argument\n\n       -p, --port  PORT\n              use PORT instead of default port\n");
            break;
        default:
            PutString("Try '");PutString(argv[0]);PutString(" --help' for more information.\n");
    }

    return EXIT_SUCCESS;
}


int ft_send(char *filename, OpenSocketId sd) {
    char *buffer = malloc(SIZE_BUFFER);
    int size;
    OpenFileId file;

    /* opening the file */
    file = Open(filename);
    if (file == NULL) {
        PutString("Error opening the file \"");PutString(filename);PutString("\".\n");
        return EXIT_FAILURE;
    }

    /* init network connection */
    Connect(TIMEOUT, sd);

    /* sending the filename */
    size = strlen(filename) + 1;
    if (!Write((char *) &size, sizeof(int), sd)) return EXIT_FAILURE; /* y'a eu une couille */
    if (!Write(filename, size, sd)) return EXIT_FAILURE; /* y'a eu une couille */

    do {
        /* reading a part of the file */
        size = Read(buffer, SIZE_BUFFER, file);
        if (size <= 0) {
            PutString("Error ");PutInt(size);PutString(" reading the file \"");PutString(filename);PutString("\".\n");
            return EXIT_FAILURE;
        }

        /* sending that part */
        if (!Write((char *) &size, sizeof(int), sd)) return EXIT_FAILURE; /* y'a eu une couille */
        if (!Write(buffer, size, sd)) return EXIT_FAILURE; /* y'a eu une couille */

    } while (size == SIZE_BUFFER);

    /* close network connection */
    Close(sd);

    Close(file);
    free(buffer);
    return EXIT_SUCCESS;
}

int ft_receive(OpenSocketId sd) {
    char *filename = malloc(SIZE_BUFFER), *buffer = malloc(SIZE_BUFFER);
    int size, res;
    remote_peer_t client;

    //for (;;) {

    /* init network connection */
    Accept(&client, 0, sd);

    /* receiving the filename */
    if (!Read((char *) &size, sizeof(int), sd)) return EXIT_FAILURE; /* y'a eu une couille */
    if (!Read(filename, size, sd)) return EXIT_FAILURE; /* y'a eu une couille */

    /* creating/opening the file */
    res = Create(filename, O_RW);
    if (res != E_SUCESS && res != E_EXIST) {  // TODO: handle proper overwrite
        PutString("Can't create \"");PutString(filename);PutString("\".\n");
        return EXIT_FAILURE;
    }
    OpenFileId file = Open(filename);
    if (file == NULL) {
        PutString("The file \"");PutString(filename);PutString("\" does not exist.\n");
        return EXIT_FAILURE;
    }

    do {
        /* receiving a part of the file */
        if (!Read((char *) &size, sizeof(int), sd)) return EXIT_FAILURE; /* y'a eu une couille */
        if (!Read(buffer, size, sd)) return EXIT_FAILURE; /* y'a eu une couille */

        /* writing that part */
        res = Write(buffer, size, file);
        if (res != size) {
            PutString("Error ");PutInt(size);PutString(" writing the file \"");PutString(filename);PutString("\".\n");
            return EXIT_FAILURE;
        }
    } while (size == SIZE_BUFFER);

    Close(file);

    /* close network connection */
    Close(sd);

    //}

    free(filename); free(buffer);
    return EXIT_SUCCESS;
}
