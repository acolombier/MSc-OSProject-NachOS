#include "transfer.h"
#include "network.h"
#include "post.h"
#include "system.h"     // for postOffice member


Connection::Connection(NetworkAddress to, MailBoxAddress mailbox) {
    toMachine = to;
    toMail = mailbox;
}

/*
Connection::~Connection() {

}
 */

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

int Connection::ReceiveFixedSize(char *data) {
    PacketHeader /* outPktHdr, */ inPktHdr;
    MailHeader /* outMailHdr, */ inMailHdr;
    /* TransferHeader outTrHdr, inTrHdr; */
    char /* outBuffer[MaxMailSize], */ inBuffer[MaxMailSize];

    postOffice->Receive(toMail, &inPktHdr, &inMailHdr, inBuffer);


    return 0;
}

/*
int Connection::Send(char *data) {

}

int Connection::Receive(char *data) {

}

int Connection::SendFile(int fd, int fileSize) {

}

int Connection::ReceiveFile(int fd, int fileSize) {

}
 */
