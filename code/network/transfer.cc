#include "network.h"
#include "post.h"

/*
Connection::Connection() {

}

Connection::~Connection() {

}
 */

int Connection::SendFixedSize(char *data) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    TransferHeader outTrHdr;
    char buffer[MaxMailSize]

    outPktHdr.to = toMachine;

    outMailHdr.to = toMail;
    outMailHdr.from = 1;
    outMailHdr.length = sizeof(TransferHeader) + strlen(data) + 1;

    outTrHdr.flags = 0;
    outTrHdr.seqNumber = ++localSeqNumber;

    postOffice->Send(outPktHdr, outMailHdr, )

    // recived ack
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
