#include "transfer.h"
#include "network.h"
#include "post.h"
#include "system.h" // for posrmt_adrffice member

Connection::Connection(MailBoxAddress localbox, NetworkAddress to, MailBoxAddress mailbox, Status s):
    _status(s), _last_local_seq_number(s == CONNECTING ? 1 : 0), _last_remote_seq_number(s == CONNECTING ? 1 : 0)
{
    lcl_box = localbox;
    rmt_adr = to;
    rmt_box = mailbox;
}

Connection::~Connection() {
    if (status() == ESTABLISHED)
        Close(TEMPO);

    postOffice->releaseBox(lcl_box);
}

bool Connection::Send(const char *data, size_t length) {
    
    ASSERT(_status == ESTABLISHED);
    
    unsigned int attempts = 0, num_chunks = divRoundUp(length, MAX_MESSAGE_SIZE);
    char chunk[MAX_MESSAGE_SIZE];
    

    for (unsigned int i = 0; i < num_chunks; i++){
        unsigned int chunk_size = i == num_chunks - 1 ? length % MAX_MESSAGE_SIZE : MAX_MESSAGE_SIZE;
        memcpy(chunk, data + (i * MAX_MESSAGE_SIZE), chunk_size);
        
        attempts = 0;
        
        while (true){
            char flag;
            if (_send_worker(0, /*timeout*/-1, chunk, chunk_size) == (int)chunk_size){
                unsigned int acked_seq;
                if ((flag = _read_worker(TEMPO, (char*)&acked_seq, 4)) == ACK && acked_seq == _last_local_seq_number){ // Need to be strict, could be a END|ACK
                    DEBUG('L', "Chunk %d is aknowledged\n", i);
                    break;
                }
                else if (flag & END){
                    DEBUG('L', "Interrupting and closing as requested\n", flag);
                    Close(/*timeout*/-1);
                    return false;
                } else if (flag != TIMEOUT){
                    DEBUG('L', "Attempt %d on %d: seq is %d and should be %d\n", attempts+1, MAXREEMISSIONS, acked_seq, _last_local_seq_number);
                    _last_local_seq_number--;
                }
            }
            if (++attempts == MAXREEMISSIONS) {
                DEBUG('L', "Connection::Send -- too many attempts\n");
                return false;
            }
        }
    }
    
    //~ while ((_last_local_seq_number - beg_seq_nb) * MAX_MESSAGE_SIZE < length) {
        //~ flags = 0;
        //~ if (beg_seq_nb == _last_local_seq_number)
            //~ flags = flags | START;
        //~ if ((_last_local_seq_number - beg_seq_nb + 1) * MAX_MESSAGE_SIZE >= length) {
            //~ flags = flags | END;
            //~ chunk_size = length - (_last_local_seq_number - beg_seq_nb) * MAX_MESSAGE_SIZE;
        //~ }

        //~ memcpy(chunk,
               //~ data + (_last_local_seq_number - beg_seq_nb) * MAX_MESSAGE_SIZE,
               //~ chunk_size);

        //~ do {
            //~ _send_worker(flags, /*timeout*/-1, chunk, chunk_size);
            //~ attempts++;
        //~ } while (!(_read_worker(TEMPO) & ACK) &&
                 //~ _last_remote_seq_number == _last_local_seq_number &&
                 //~ attempts < MAXREEMISSIONS);

        //~ if (attempts == MAXREEMISSIONS) {
            //~ DEBUG('L', "Connection::Send -- too many attempts\n");
            //~ return false;
        //~ }

        //~ _last_local_seq_number++;
    //~ }
    //~ _send_worker(ACK, /*timeout*/-1);
    return true;
}

bool Connection::Receive(char *data, size_t length) {
    
    ASSERT(_status == ESTABLISHED);
    
    unsigned int attempts = 0, num_chunks = divRoundUp(length, MAX_MESSAGE_SIZE), starting = _last_remote_seq_number;
    char chunk[MAX_MESSAGE_SIZE];
    
    
    while (true){        
        if (_last_remote_seq_number - starting == num_chunks)
            return true;
            
        unsigned int current_chunk = _last_remote_seq_number - starting, 
                     chunk_size = current_chunk == num_chunks - 1 ? length % MAX_MESSAGE_SIZE : MAX_MESSAGE_SIZE;
        
        attempts = 0;
        
        while (true){
            char flag;
            if ((flag = _read_worker(/*timeout*/-1, chunk, chunk_size)) == 0){ // No flag. What about if it is an END ? We should close the connection
                if (_send_worker(ACK, /*timeout*/TEMPO, (char*)&_last_remote_seq_number, sizeof(int)) == 0){
                    DEBUG('L', "Chunk %d is been aknowledged\n", (current_chunk));
                    memcpy(data + (current_chunk * MAX_MESSAGE_SIZE), chunk, chunk_size);
                    break;
                }
            } else if (flag & END){
                DEBUG('L', "Interrupting and closing as requested\n", flag);
                Close(/*timeout*/-1);
                return false;
            } else if (flag != TIMEOUT){
                DEBUG('L', "Attempt %d on %d failed: after acking seq is %d\n", _last_remote_seq_number); 
                _last_remote_seq_number--;
            }           
            if (++attempts == MAXREEMISSIONS) {
                DEBUG('L', "Connection::Send -- too many attempts\n");
                return false;
            }
        }
    }
    return true;
    
    //~ unsigned int attempts = 1, beg_seq_nb = _last_remote_seq_number;
    //~ char flags, chunk[MAX_MESSAGE_SIZE];
    //~ size_t chunk_size = MAX_MESSAGE_SIZE;

    //~ do {
        //~ flags = _read_worker(/*timeout*/-1, chunk, MAX_MESSAGE_SIZE);
        //~ _send_worker(flags | ACK, /*timeout*/-1);

        //~ if (flags & START)
            //~ beg_seq_nb = _last_remote_seq_number;
        //~ if (flags & END) {
            //~ chunk_size = length - (_last_local_seq_number - beg_seq_nb) * MAX_MESSAGE_SIZE;
            //~ if (chunk_size > MAX_MESSAGE_SIZE) chunk_size = MAX_MESSAGE_SIZE;
        //~ }

        //~ memcpy(data + (_last_remote_seq_number - beg_seq_nb) * MAX_MESSAGE_SIZE,
               //~ chunk, chunk_size);
    //~ } while (!(flags & END) && length >= (_last_local_seq_number - beg_seq_nb + 1) * MAX_MESSAGE_SIZE);

    //~ do {
        //~ flags = _read_worker(TEMPO);
        //~ if (flags & ACK) break;
        //~ _send_worker(END | ACK, /*timeout*/-1);
        //~ attempts++;
    //~ } while (attempts < MAXREEMISSIONS);
    //~ return true;
}

bool Connection::Accept(int timeout){
    ASSERT(_status == IDLE);

    unsigned long start_time = stats->totalTicks, current_time;
    
    unsigned int attempts;
    
    _status = ACCEPTING;

    do {
        NetworkAddress remoteAddr;
        MailBoxAddress remotePort;
        char flags;
        _last_local_seq_number =  0;
        _last_remote_seq_number = 0;
        
        attempts = 0;        
        do {
            current_time = stats->totalTicks - start_time;
            if ((flags = _read_worker(timeout == -1 ? -1 : timeout - current_time, nullptr, 0,
                    &remoteAddr, &remotePort)) == 0 && flags == START) //We strongly want only a start flag, otherwise we consider it as flood
                break;
            
        } while (++attempts < MAXREEMISSIONS);
        
        rmt_adr = remoteAddr;
        rmt_box = remotePort;
        attempts = 0;
        do {
            current_time = stats->totalTicks - start_time;
            if (_send_worker(START | ACK, timeout == -1 ? -1 : timeout - current_time) == 0) // Timeout
                break;
        } while (++attempts < MAXREEMISSIONS);

        attempts = 0;
        while (true) {
            current_time = stats->totalTicks - start_time;             
            if (_read_worker(timeout) == ACK){
                _status = ESTABLISHED;
                return true;
            }
            else if (++attempts == MAXREEMISSIONS || current_time > (unsigned int)timeout)
                break;
        }
    } while (timeout < 0 || current_time < (unsigned int)timeout);
    
    rmt_adr = -1;
    rmt_box = -1;
    _status = IDLE;

    return false;
}

bool Connection::Connect(int timeout){
    ASSERT(_status == IDLE);

    unsigned long start_time, emission_time, emission_max_time = timeout == -1 ? -1 : timeout / MAXREEMISSIONS;
    unsigned int attempts = 0;
    
    _status = CONNECTING;

    do {
        char flags;
        start_time = stats->totalTicks;
        emission_time = 0;
        
        if (_send_worker(START, timeout == -1 ? -1 : emission_max_time - emission_time) != 0) // Timeout
            continue;
        
            
        emission_time = stats->totalTicks - start_time;
        if ((flags = _read_worker(timeout == -1 ? -1 : emission_max_time - emission_time)) < 0) // Timeout
            continue;
            
        attempts = 0;   

        emission_time = stats->totalTicks - start_time;
        if (flags != (START | ACK)){ // We strongly want only a start +ack flag, otherwise we abort. We should maybe try again
            continue;
        }
        
        emission_time = stats->totalTicks - start_time;
        if (_send_worker(ACK, timeout == -1 ? -1 : emission_max_time - emission_time) != 0) // Timeout
            continue;
            
        _status = ESTABLISHED;
        return true;
    } while (++attempts < MAXREEMISSIONS);
    
    return false;
}

bool Connection::Close(int timeout){
    ASSERT(_status == ESTABLISHED);
    
    unsigned long start_time, emission_time, emission_max_time = timeout == -1 ? -1 : timeout / MAXREEMISSIONS;
    unsigned int attempts = 0;
    
    _status = CLOSING;

    do {
        char flags;
        start_time = stats->totalTicks;
        emission_time = 0;
        
        if (_send_worker(END, emission_max_time) == 0) // Timeout
            continue;
            
        emission_time = stats->totalTicks - start_time;
        if ((flags = _read_worker(emission_max_time - emission_time)) < 0) // Timeout
            continue;

        emission_time = stats->totalTicks - start_time;
        if (flags != (END | ACK)) // We strongly want only a start +ack flag, otherwise we abort. We should maybe try again
            continue;
        
        if (_send_worker(END | ACK, emission_max_time) == 0) // Timeout
            continue;
            
        emission_time = stats->totalTicks - start_time;
        if ((flags = _read_worker(emission_max_time - emission_time)) < 0) // Timeout
            continue;
                
        DEBUG('N', "Disconnect has got success\n");
        
        _status = CLOSED;
        
        return true;
    } while (++attempts < MAXREEMISSIONS);
    
    return false;
}

int Connection::_send_worker(char flags, int timeout, char* data, size_t length){
    ASSERT(length <= MAX_MESSAGE_SIZE);

    PacketHeader outPktHdr(rmt_adr), inPktHdr;
    MailHeader outMailHdr(rmt_box, lcl_box, sizeof(TransferHeader) + length), inMailHdr;
    TransferHeader outTrHdr(flags, ++_last_local_seq_number), inTrHdr;

    char outBuffer[MaxMailSize];

    ASSERT(outMailHdr.length <= MaxMailSize);

    /* concatenate TransferHeader and data */
    memset(outBuffer, 0, MAX_MESSAGE_SIZE + sizeof(TransferHeader));
    memcpy(outBuffer, &outTrHdr, sizeof(TransferHeader));
    memcpy(outBuffer + sizeof(TransferHeader), data, length);

    ;

    /*! \todo timeout handling */

    //~ DEBUG('n', "SendPacket -- Sending \"%s\" to %d, box %d\n", outBuffer + sizeof(TransferHeader), outPktHdr.to, outMailHdr.to);

    return postOffice->Send(outPktHdr, outMailHdr, outBuffer, timeout) ? length : -1;
}

char Connection::_read_worker(int timeout, char* data, size_t length, NetworkAddress* remoteAddr, MailBoxAddress* remotePort){

    ASSERT(length <= MAX_MESSAGE_SIZE);

    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    TransferHeader inTrHdr;

    char inBuffer[MaxMailSize];

    if (!postOffice->Receive(lcl_box, &inPktHdr, &inMailHdr, inBuffer, timeout)){
        DEBUG('N', "ReceivePacket -- Timeout\n");
        return 0b1000;
    } else {
        memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));
        memcpy(data, inBuffer + sizeof(TransferHeader), inMailHdr.length < length ? inMailHdr.length : length);
                                                      // `-> inMailHdr.length?
        _last_remote_seq_number = inTrHdr.seq_num
        ;
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
