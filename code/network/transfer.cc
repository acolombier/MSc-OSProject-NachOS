#include "transfer.h"
#include "network.h"
#include "post.h"
#include "system.h"     // for postOffice member


Connection::Connection(NetworkAddress to, MailBoxAddress mailbox) {
    toMachine = to;
    toMail = mailbox;
}

Connection::~Connection() {

}

int Connection::SendFixedSize(char *data, char flags) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    TransferHeader outTrHdr, inTrHdr;
    char outBuffer[MaxMailSize], inBuffer[MaxMailSize];
    int attempts = 0;

    /* link + transfer layers */
    outPktHdr.to = toMachine;

    outMailHdr.to = toMail;
    outMailHdr.from = 1;
    outMailHdr.length = sizeof(struct TransferHeader) + strlen(data) + 1;

    /* application layer */
    outTrHdr.flags = flags;

    /* concatenate TransferHeader and data */
    memcpy(outBuffer, &outTrHdr, sizeof(struct TransferHeader));
    memcpy(outBuffer + sizeof(struct TransferHeader), data, MAX_MESSAGE_SIZE);

    do {
        postOffice->Send(outPktHdr, outMailHdr, outBuffer);

        /* recieve acknowledgement */
        postOffice->Receive(toMail, &inPktHdr, &inMailHdr, inBuffer, TEMPO);

        /* unpack the data of the mail that gets put into inBuffer
            which consists of TransferHeader + data */
        memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));

        attempts++;
    } while (!(inTrHdr.flags & (flags | ACK)) && attempts < MAXREEMISSIONS);

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
    DEBUG('n', "Got \"%s\" from %d, box %d\n", inBuffer + 1, inPktHdr.from, inMailHdr.from);

    memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));
    memcpy(data, &inBuffer + sizeof(TransferHeader), inMailHdr.length - sizeof(TransferHeader));

    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = sizeof(TransferHeader);
    outTrHdr.flags = 0 | ACK;
    postOffice->Send(outPktHdr, outMailHdr, (char *) &outTrHdr);

    return inTrHdr.flags;
}

int Connection::Send(char *data) {
    unsigned int offset = 0;
    char chunk[MAX_MESSAGE_SIZE], flags;

    while (offset < strlen(data) + 1) {
        flags = 0;
        if (offset == 0)
            flags = flags | START;
        if (offset + MAX_MESSAGE_SIZE >= strlen(data) + 1)
            flags = flags | END;
        strncpy(chunk, data + offset, MAX_MESSAGE_SIZE);

        if (SendFixedSize(chunk, flags) == -1)
            return -1;

        offset += MAX_MESSAGE_SIZE;
    }

    return 0;
}

int Connection::Receive(char *data) {
    unsigned int offset = 0;
    char chunk[MAX_MESSAGE_SIZE], flags;

    do {
        flags = ReceiveFixedSize(chunk);
        memcpy(data + offset, chunk, MAX_MESSAGE_SIZE);
        offset += MAX_MESSAGE_SIZE;
    } while (!(flags & END));

    return 0;
}

/* int Connection::SendFile(int fd, int fileSize) {

}

int Connection::ReceiveFile(int fd, int fileSize) {

} */
