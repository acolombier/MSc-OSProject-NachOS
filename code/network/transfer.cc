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

bool Connection::Send(char *data) {
    unsigned int attempts = 0;
    do {
        /*! \todo implementation */
        attempts++;
    } while (attempts < MAXREEMISSIONS);

    if (attempts == MAXREEMISSIONS) {
        DEBUG('N', "Connection::Receive -- too many attempts");
        return false;
    } else
        return true;
}

bool Connection::Receive(char *data) {
    unsigned int attempts = 0;
    do {
        /*! \todo implementation */
        attempts++;
    } while (attempts < MAXREEMISSIONS);

    if (attempts == MAXREEMISSIONS) {
        DEBUG('N', "Connection::Receive -- too many attempts");
        return false;
    } else
        return true;
}

Connection* Connection::Accept(int timeout){
    /*! \todo implementation */
    return nullptr;
}

bool Connection::Connect(int timeout){
    /*! \todo implementation */
    return false;
}

bool Connection::Close(int timeout){
    /*! \todo implementation */
    return false;
}

int Connection::_send_worker(char flags, int timeout, char* data, size_t length){
    ASSERT(length <= MAX_MESSAGE_SIZE);
    
    PacketHeader outPktHdr(rmt_adr), inPktHdr;
    MailHeader outMailHdr(rmt_box, lcl_box, sizeof(TransferHeader) + length), inMailHdr;
    TransferHeader outTrHdr(flags, _last_local_seq_number++), inTrHdr;
    
    char outBuffer[MaxMailSize];

    ASSERT(outMailHdr.length <= MaxMailSize);

    /* concatenate TransferHeader and data */
    memset(outBuffer, 0, MAX_MESSAGE_SIZE + sizeof(TransferHeader));

    postOffice->Send(outPktHdr, outMailHdr, outBuffer);
    /* timeout handling */
    DEBUG('n', "SendPacket -- Sending \"%s\" to %d, box %d\n", outBuffer + sizeof(TransferHeader), outPktHdr.to, outMailHdr.to);
    
    return 0;
}

char Connection::_read_worker(int timeout, char* data, size_t length){
    
    ASSERT(length <= MAX_MESSAGE_SIZE);
    
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    TransferHeader inTrHdr;
    
    char inBuffer[MaxMailSize];

    if (!postOffice->Receive(lcl_box, &inPktHdr, &inMailHdr, inBuffer, timeout)){
        DEBUG('N', "ReceivePacket -- Looks like we got a timeout");
        return -1;
    } else {        
        memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));
        if (inTrHdr.seq_num != _last_remote_seq_number){
            /* we probably missed something */
        } else
            memcpy(data, inBuffer + sizeof(TransferHeader), length);
        return 0;
    }
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
