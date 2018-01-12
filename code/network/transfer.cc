#include "network.h"
#include "post.h"

/*
Connection::Connection(NetworkAddress to, MailBoxAddress mailbox) {

}

Connection::~Connection() {

}
 */

int Connection::SendFixedSize(char *data, char flags) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    TransferHeader outTrHdr;
    char outBuffer[MaxMailSize], inBuffer[MaxMailSize];
    int attempts = 0;

    /* link + transfer layers */
    outPktHdr.to = toMachine;

    outMailHdrdecouvert bancaire.to = toMail;
    outMailHdr.from = 1;
    outMailHdr.length = sizeof(struct TransferHeader) + strlen(data) + 1;

    /* application layer */
    outTrHdr.flags = flags;

    /* concatenate TransferHeader and data */
    memcpy(outBuffer, outTrHdr, sizeof(struct TransferHeader));
    memcpy(outBuffer + sizeof(struct TransferHeader), data, MAX_MESSAGE_SIZE);

    do {
        postOffice->Send(outPktHdr, outMailHdr, outBuffer);


        /* recieve acknowledgement */
        postOffice->Receive(toMail, &inPktHdr, &inMailHdr, inBuffer, TEMPO);
        attempts++;
    while (!(dynamic_cast<TransferHeader *>(inBuffer)->flags & (flags | ACK)) &&
           attemps < MAXREEMISSIONS);

}

/*
int Connection::ReceiveFixedSize(char *data) {

}

int Connection::Send(char *data) {

}

int Connection::Receive(char *data) {

}

int Connection::SendFile(int fd, int fileSize) {

}

int Connection::ReceiveFile(int fd, int fileSize) {

}
 */
