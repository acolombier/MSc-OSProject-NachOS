#include "transfer.h"
#include "network.h"
#include "post.h"
#include "system.h" // for posrmt_adrffice member

Connection::Connection(MailBoxAddress localbox, NetworkAddress to, MailBoxAddress mailbox) {
    lcl_box = localbox;
    rmt_adr = to;
    rmt_box = mailbox;
}

Connection::~Connection() {
    if (status() == CLOSED || status() == CLOSING)
        Close(TEMPO);
}

/* Depreciated */
int Connection::SendFixedSize(char *data, size_t length, unsigned int seq_num, char flags) {
    ASSERT(length <= MAX_MESSAGE_SIZE);
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    TransferHeader outTrHdr, inTrHdr;
    char outBuffer[MaxMailSize], inBuffer[MaxMailSize];
    int attempts = 0;

    memset(&outPktHdr, 0, sizeof(PacketHeader));
    memset(&inPktHdr, 0, sizeof(PacketHeader));
    memset(&outMailHdr, 0, sizeof(MailHeader));
    memset(&inMailHdr, 0, sizeof(MailHeader));
    memset(&outTrHdr, 0, sizeof(TransferHeader));
    memset(&inTrHdr, 0, sizeof(TransferHeader));

    outPktHdr.to = rmt_adr;
    outMailHdr.to = rmt_box;
    outMailHdr.from = lcl_box;
    outMailHdr.length = sizeof(TransferHeader) + length;
    ASSERT(outMailHdr.length <= MaxMailSize);

    outTrHdr.seq_num = seq_num;
    outTrHdr.flags = flags;

    /* concatenate TransferHeader and data */
    memcpy(outBuffer, &outTrHdr, sizeof(TransferHeader));
    memcpy(outBuffer + sizeof(TransferHeader), data, MAX_MESSAGE_SIZE);

    do {
        postOffice->Send(outPktHdr, outMailHdr, outBuffer);
        DEBUG('n', "Connection::SendFixedSize -- Sending fixed \"%s\" to %d, box %d\n", outBuffer + sizeof(TransferHeader), outPktHdr.to, outMailHdr.to);

        /* recieve acknowledgement */
        DEBUG('n', "Connection::SendFixedSize -- waiting for ACK\n");
        postOffice->Receive(lcl_box, &inPktHdr, &inMailHdr, inBuffer, TEMPO);
        if (inBuffer == NULL) {
            DEBUG('n', "Connection::SendFixedSize -- did not got ACK\n");
        } else {
            DEBUG('n', "Connection::SendFixedSize -- got ACK from %d, box %d\n", inPktHdr.from, inMailHdr.from);

            /* unpack the data of the mail that gets put into inBuffer
                which consists of TransferHeader + data */
            memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));
            //DEBUG('n', "Connection::SendFixedSize -- receive flags %s\n", flagstostr(inTrHdr.flags));
        }

        attempts++;
    } while (!(inTrHdr.seq_num == seq_num && inTrHdr.flags & ACK) &&
             attempts < MAXREEMISSIONS);

    if (attempts == MAXREEMISSIONS) {
        DEBUG('N', "Connection::SendFixedSize -- too many attempts");
        return -1;
    } else {
        return 0;
    }
}

/* Depreciated */
void Connection::ReceiveFixedSize(char *data) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    TransferHeader outTrHdr, inTrHdr;
    char inBuffer[MaxMailSize];

    memset(&outPktHdr, 0, sizeof(PacketHeader));
    memset(&inPktHdr, 0, sizeof(PacketHeader));
    memset(&outMailHdr, 0, sizeof(MailHeader));
    memset(&inMailHdr, 0, sizeof(MailHeader));
    memset(&outTrHdr, 0, sizeof(TransferHeader));
    memset(&inTrHdr, 0, sizeof(TransferHeader));

    postOffice->Receive(rmt_box, &inPktHdr, &inMailHdr, inBuffer);
    DEBUG('n', "Connection::ReceiveFixedSize -- got \"%s\" from %d, box %d\n",
          inBuffer + sizeof(TransferHeader), inPktHdr.from, inMailHdr.from);

    memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));
    memcpy(data, inBuffer, inMailHdr.length);

    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = lcl_box;
    outMailHdr.length = sizeof(TransferHeader);
    outTrHdr.seq_num = inTrHdr.seq_num;
    outTrHdr.flags = inTrHdr.flags | ACK;
    DEBUG('n', "Connection::ReceiveFixedSize -- sending ACK to %d, box %d\n", outPktHdr.to, outMailHdr.to);
    postOffice->Send(outPktHdr, outMailHdr, (char *) &outTrHdr);
}

int Connection::Send(char *data) {
    unsigned int seq_num = 0;
    char chunk[MAX_MESSAGE_SIZE], flags;

    while (seq_num * MAX_MESSAGE_SIZE < strlen(data) + 1) {
        flags = 0;
        if (seq_num == 0)
            flags = flags | START;
        if ((seq_num + 1) * MAX_MESSAGE_SIZE >= strlen(data) + 1)
            flags = flags | END;
        memcpy(chunk, data + (seq_num * MAX_MESSAGE_SIZE), MAX_MESSAGE_SIZE);

        DEBUG('n', "Connection::Send -- Send packet %d of %d\n", seq_num, (strlen(data) + 1) / MAX_MESSAGE_SIZE);
        if (SendFixedSize(chunk, MAX_MESSAGE_SIZE, seq_num, flags) == -1)
            return -1;
        seq_num++;
    }
    return 0;
}

void Connection::Receive(char *data) {
    char chunk[MaxMailSize];
    TransferHeader inTrHdr;

    do {
        ReceiveFixedSize(chunk);
        //DEBUG('n', "Current flags at receive are: %s\n", flagstostr(flags));
        memset(&inTrHdr, 0, sizeof(TransferHeader));
        memcpy(&inTrHdr, chunk, sizeof(TransferHeader));
        memcpy(data + (inTrHdr.seq_num * MAX_MESSAGE_SIZE), chunk + sizeof(TransferHeader), MAX_MESSAGE_SIZE);
    } while (!(inTrHdr.flags & END));
}

Connection* Accept(int timeout){
}

bool Connect(int timeout){
}

bool Close(int timeout){
}

int _send_worker(char flags, int timeout, char* data, size_t lenght){
}

char _read_worker(int timeout, char* data, size_t lenght){
}

char* Connection::flagstostr(char flags) {
    int offset = 0;
    char *str = new char[(flags & ACK ? 4 : 0) + (flags & START ? 6 : 0) + (flags & END ? 4 : 0)];

    if (flags & ACK){
        strcpy(str + offset, "ACK|");
        offset += 3;
    }

    if (flags & START){
        strcpy(str + offset, "START|");
        offset += 5;
    }

    if (flags & END){
        strcpy(str + offset, "END|");
        offset += 3;
    }
    
    str[offset] = '\0';
    
    return str;
}
