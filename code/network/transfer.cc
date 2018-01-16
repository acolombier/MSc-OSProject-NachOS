#include "transfer.h"
#include "network.h"
#include "post.h"
#include "system.h" // for posrmt_adrffice member

Connection::Connection(MailBoxAddress localbox, NetworkAddress to, MailBoxAddress mailbox, Status s):
    _status(s),_last_local_seq_number(s == CONNECTING ? 1 : 0), _last_remote_seq_number(s == CONNECTING ? 1 : 0)
{
    lcl_box = localbox;
    rmt_adr = to;
    rmt_box = mailbox;
}

Connection::~Connection() {
    if (status() == CLOSED || status() == CLOSING)
        Close(TEMPO);
        
    postOffice->releaseBox(lcl_box);
}

bool Connection::Send(char *data, size_t length) {
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

bool Connection::Receive(char *data, size_t length) {
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
    ASSERT(_status == IDLE);
    
    unsigned long start_time = stats->totalTicks, shift_time = 0;
    Connection* new_conn = nullptr;
    
    do {
        NetworkAddress remoteAddr;
        MailBoxAddress remotePort;
        char flags;
        
        shift_time += stats->totalTicks - start_time;
        if ((flags = _read_worker(timeout - shift_time, nullptr, 0, 
                &remoteAddr, &remotePort)) < 0) // Timeout
            break;
            
        shift_time += stats->totalTicks - start_time;
        if (flags != START) //We strongly want only a start flag, otherwise we consider it as flood
            continue;
        
        new_conn = new Connection(postOffice->assignateBox(), remoteAddr, 
            remotePort, CONNECTING);
            
        if (new_conn->Synch(timeout - shift_time)){ /*! \todo retried if failed */
            delete new_conn;
            break;
        }
            
        return new_conn; // From now, we assume that the connection as syncronysed
    } while (timeout < 0 || shift_time < (unsigned int)timeout);

    return nullptr;
}

bool Connection::Connect(int timeout){
    ASSERT(_status == IDLE);
    /*! \todo implementation */
    return false;
}

bool Connection::Synch(int timeout){
    if (!_read_worker(timeout))
        return false;
    _last_remote_seq_number++;
    _status = ESTABLISHED;
    return true;
}


bool Connection::Close(int timeout){
    ASSERT(_status == ESTABLISHED);
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
    
    /*! \todo timeout handling */
    
    DEBUG('n', "SendPacket -- Sending \"%s\" to %d, box %d\n", outBuffer + sizeof(TransferHeader), outPktHdr.to, outMailHdr.to);
    
    return 0;
}

char Connection::_read_worker(int timeout, char* data, size_t length, NetworkAddress* remoteAddr, MailBoxAddress* remotePort){
    
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
            return -1;
        } else
            memcpy(data, inBuffer + sizeof(TransferHeader), length);
            
        if (remoteAddr)
            *remoteAddr = inPktHdr.from;
        if (remotePort)
            *remotePort = inMailHdr.from;
            
        return inTrHdr.flags;
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
