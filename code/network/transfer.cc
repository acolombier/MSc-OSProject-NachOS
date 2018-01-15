#include "transfer.h"
#include "network.h"
#include "post.h"
#include "system.h"     // for postOffice member


Connection::Connection(MailBoxAddress localbox,
                       NetworkAddress to, MailBoxAddress mailbox) {
    fromMail = localbox;
    toMachine = to;
    toMail = mailbox;
}

Connection::~Connection() {

}

int Connection::SendFixedSize(char *data, char flags) {
    ASSERT(strlen(data) <= MAX_MESSAGE_SIZE - 1);
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    TransferHeader outTrHdr, inTrHdr;
    char outBuffer[MaxMailSize], inBuffer[MaxMailSize];
    int attempts = 0;

    /* link + transfer layers */
    memset(&inMailHdr, 0, sizeof(MailHeader));
    memset(&outPktHdr, 0, sizeof(PacketHeader));
    outPktHdr.to = toMachine;

    outMailHdr.to = toMail;
    outMailHdr.from = fromMail;
    outMailHdr.length = sizeof(TransferHeader) + strlen(data) + 1;
    ASSERT(outMailHdr.length <= MaxMailSize);

    /* application layer */
    outTrHdr.flags = flags;

    /* concatenate TransferHeader and data */
    memcpy(outBuffer, &outTrHdr, sizeof(TransferHeader));
    memcpy(outBuffer + sizeof(TransferHeader), data, MAX_MESSAGE_SIZE); // added '/0' included

    do {
        postOffice->Send(outPktHdr, outMailHdr, outBuffer);
        DEBUG('n', "Sending fixed \"%s\" to %d, box %d\n", outBuffer + sizeof(TransferHeader), outPktHdr.to, outMailHdr.to);

        /* recieve acknowledgement */
        DEBUG('n', "Sending fixed: waiting for ACK\n");
        postOffice->Receive(fromMail, &inPktHdr, &inMailHdr, inBuffer, TEMPO);
        if (inBuffer != NULL) {
            DEBUG('n', "Sending fixed: got ACK from %d, box %d\n", inPktHdr.from, inMailHdr.from);
            fflush(stdout);

            /* unpack the data of the mail that gets put into inBuffer
                which consists of TransferHeader + data */
            memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));
            //DEBUG('n', "Sending fixed: receive flags %s\n", flagstostr(inTrHdr.flags));
        } else {
            DEBUG('n', "Sending fixed: did not got ACK\n");
            fflush(stdout);
            inTrHdr.flags = 0;
        }

        attempts++;
    } while (!(inTrHdr.flags & (flags | ACK)) && attempts < MAXREEMISSIONS);

    DEBUG('n', "Came out of the sending loop.\n");

    if (attempts == MAXREEMISSIONS)
        return -1;
    else
        return 0;
}

char Connection::ReceiveFixedSize(char *data) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    TransferHeader outTrHdr, inTrHdr;
    char inBuffer[MaxMailSize];

    postOffice->Receive(toMail, &inPktHdr, &inMailHdr, inBuffer);
    DEBUG('n', "Our receive got \"%s\" from %d, box %d\n",
               inBuffer + sizeof(TransferHeader), inPktHdr.from, inMailHdr.from);

    memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));
    memcpy(data, inBuffer + sizeof(TransferHeader), inMailHdr.length - sizeof(TransferHeader));

    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = fromMail;
    outMailHdr.length = sizeof(TransferHeader);
    outTrHdr.flags = inTrHdr.flags | ACK;
    DEBUG('n', "Our receive sending ACK to %d, box %d\n", outPktHdr.to, outMailHdr.to);
    postOffice->Send(outPktHdr, outMailHdr, (char *) &outTrHdr);

    return inTrHdr.flags;
}

int Connection::Send(char *data) {
    unsigned int offset = 0;
    char chunk[MAX_MESSAGE_SIZE], flags;

    while (offset < strlen(data)) {
        flags = 0;
        if (offset == 0)
            flags = flags | START;
        if (offset + MAX_MESSAGE_SIZE - 1 >= strlen(data))
            flags = flags | END;
        strncpy(chunk, data + offset, MAX_MESSAGE_SIZE - 1);
        chunk[MAX_MESSAGE_SIZE - 1] = '\0';

        DEBUG('n', "--> Send char at offset %d in str of len %d\n", offset, strlen(data) + 1);
        if (SendFixedSize(chunk, flags) == -1)
            return -1;

        offset += MAX_MESSAGE_SIZE - 1;
    }

    return 0;
}

int Connection::Receive(char *data) {
    unsigned int offset = 0;
    char chunk[MAX_MESSAGE_SIZE], flags;

    do {
        flags = ReceiveFixedSize(chunk);
        //DEBUG('n', "Current flags at receive are: %s\n", flagstostr(flags));
        // memcpy: put data to correct place; copy without headers; copy without artificially added '\0'
        memcpy(data + offset, chunk, MAX_MESSAGE_SIZE - 1);
        offset += MAX_MESSAGE_SIZE - 1;
    } while (!(flags & END));

    return 0;
}

/* int Connection::SendFile(int fd, int fileSize) {

}

int Connection::ReceiveFile(int fd, int fileSize) {

} */

char *flagstostr(char flags) {
    char *str = (char *) malloc(14 * sizeof(char));
    str[0] = '\0';

    if (flags & ACK)
        strcat(str, "ACK ");

    if (flags & START)
        strcat(str, "START ");

    if (flags & END)
        strcat(str, "END ");

    return str;
}
