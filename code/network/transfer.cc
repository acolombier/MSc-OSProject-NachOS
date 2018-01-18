#include "transfer.h"
#include "network.h"
#include "post.h"
#include "system.h" // for posrmt_adrffice member

Connection::Connection(MailBoxAddress localbox, NetworkAddress to, MailBoxAddress mailbox, Status s):
    _status(s), _last_local_seq_number(s == CONNECTING ? 1 : 0), _last_remote_seq_number(s == CONNECTING ? 1 : 0), _lock(new Lock("Connection"))
{
    lcl_box = localbox;
    rmt_adr = to;
    rmt_box = mailbox;
}

Connection::~Connection() {
    if (status() == ESTABLISHED)
        Close(TEMPO);

    delete _lock;
    
    postOffice->registerCloseHandler(lcl_box, nullptr);
    postOffice->releaseBox(lcl_box);
}

bool Connection::Send(const char *data, size_t length) {
    
    ASSERT(_status == ESTABLISHED);
    
    unsigned int attempts = 0, num_chunks = divRoundUp(length, MAX_MESSAGE_SIZE);
    char chunk[MAX_MESSAGE_SIZE];
    

    for (unsigned int i = 0; i < num_chunks && _status == ESTABLISHED;){
        unsigned int chunk_size = i == num_chunks - 1 ? length % MAX_MESSAGE_SIZE : MAX_MESSAGE_SIZE;
        memcpy(chunk, data + (i * MAX_MESSAGE_SIZE), chunk_size);
        
        attempts = 0;
        
        _lock->Acquire();
        while (_status == ESTABLISHED){
            char flag;
            _last_local_seq_number = i;
            if (_send_worker(0, TEMPO, chunk, chunk_size) == (int)chunk_size){
                unsigned int acked_seq;
                if ((flag = _read_worker(TEMPO, (char*)&acked_seq, 4)) == ACK && acked_seq == _last_local_seq_number){ // Need to be strict, could be a END|ACK
                    DEBUG('L', "Chunk %d is aknowledged (data=%p) (seq=%d)\n", i, (void*)(((int*)chunk)[0]), _last_local_seq_number);
                    i++;
                    break;
                } else if (flag == (START | ACK)){
                    DEBUG('L', "Connection re-synch\n");
                    _send_worker(ACK, TEMPO);
                    i = 0;
                    break;
                } else if (flag == RESET){
                    DEBUG('L', "Connection reset\n");
                    _status = IDLE;
                    Connect(TEMPO);
                    i = 0;
                    break;
                } else if (flag != TIMEOUT){
                    DEBUG('L', "Attempt %d on %d: seq is %d and should be %d\n", attempts+1, MAXREEMISSIONS, acked_seq, _last_local_seq_number);
                }
                //~ _last_local_seq_number--;
            }
            if (++attempts == MAXREEMISSIONS) {
                DEBUG('L', "Connection::Send -- too many attempts\n");
                _lock->Release();
                return false;
            }
        }
        _lock->Release();
    }
    return _status == ESTABLISHED;
}

bool Connection::Receive(char *data, size_t length) {
    
    ASSERT(_status == ESTABLISHED);
    
    unsigned int attempts = 0, current_chunk = 0, num_chunks = (unsigned int)divRoundUp(length, MAX_MESSAGE_SIZE), starting = 1,
        chunk_size = MAX_MESSAGE_SIZE;;
    char chunk[MAX_MESSAGE_SIZE];
    
    memset(data, 0, length);
    
    
    DEBUG('L', "Connection::Receive -- %d to receive\n", num_chunks);
    
    while (true){        
        if (current_chunk == num_chunks - 1) // If it's the last chunk, the size might be smaller
            chunk_size = length % MAX_MESSAGE_SIZE;
        
        if (current_chunk == num_chunks){
            attempts = 0;
            do {
                if (_read_worker(SYNC_TEMPO) != TIMEOUT)
                    break;
            } while(++attempts == MAXREEMISSIONS);
            if (attempts == MAXREEMISSIONS || _last_remote_seq_number == num_chunks)
                return true;
        }
        
        attempts = 0;
        
        DEBUG('L', "Connection::Receive -- Receiving %d...\n", current_chunk);
        _lock->Acquire();
        while (_status == ESTABLISHED){
            char flag;
            if ((flag = _read_worker(TEMPO, chunk, chunk_size)) == 0){ // No flag. What about if it is an END ? We should close the connection
                if (_send_worker(ACK, TEMPO, (char*)&_last_remote_seq_number, sizeof(int)) == sizeof(int)){
                    memcpy(data + (current_chunk * MAX_MESSAGE_SIZE), chunk, chunk_size);
                    DEBUG('L', "Chunk %d on %d is been aknowledged (data=%p) (seq=%d), next is %d\n", current_chunk, num_chunks, (void*)(((int*)(data + (current_chunk * MAX_MESSAGE_SIZE)))[0]), _last_remote_seq_number, _last_remote_seq_number - starting);
                    current_chunk = _last_remote_seq_number;
                    break;
                } else
                    DEBUG('L', "Connection::Receive -- ACK -> Timeout\n"); 
            } else if (flag != TIMEOUT){
                DEBUG('L', "Attempt %d on %d failed: after acking seq is %d\n", attempts+1, MAXREEMISSIONS, _last_remote_seq_number); 
                _last_remote_seq_number--;
            } else if (current_chunk == num_chunks){
                _lock->Release();
                return true;
            }
            if (++attempts == MAXREEMISSIONS) {
                DEBUG('L', "Connection::Receive -- too many attempts\n");
                _lock->Release();
                return false;
            }
        }
        _lock->Release();
        if (_status != ESTABLISHED)
            break;
    }
    return _status == ESTABLISHED && current_chunk == num_chunks;
}

bool Connection::Accept(int timeout){
    ASSERT(_status == IDLE);

    unsigned long start_time = stats->totalTicks, current_time;
    
    unsigned int attempts;
    
    _status = ACCEPTING;

    _lock->Acquire();
    do {
        NetworkAddress remoteAddr;
        MailBoxAddress remotePort;
        char flags;
        
        attempts = 0;      
        _last_local_seq_number =  0;
          
        do {
            current_time = stats->totalTicks - start_time;
            if ((flags = _read_worker(timeout == -1 ? -1 : timeout - current_time, nullptr, 0,
                    &remoteAddr, &remotePort)) == START) //We strongly want only a start flag, otherwise we consider it as flood
                break;
            else {
                rmt_adr = remoteAddr;
                rmt_box = remotePort;
                DEBUG('L', "Peer sending data without synch -- reset request\n");
                _last_local_seq_number--;
                _send_worker(RESET, TEMPO);
            }
            
        } while (++attempts < MAXREEMISSIONS);
        DEBUG('L', "Connection::Accept -- START\n");
        
        rmt_adr = remoteAddr;
        rmt_box = remotePort;
        _last_local_seq_number =  0;
        _last_remote_seq_number = 1;
        attempts = 0;
        do {
            current_time = stats->totalTicks - start_time;
            if (_send_worker(START | ACK, SYNC_TEMPO) == 0) // Timeout
                break;
        } while (++attempts < MAXREEMISSIONS);
        DEBUG('L', "Connection::Accept -- START|ACK\n");

        attempts = 0;
        while (true) {
            current_time = stats->totalTicks - start_time; 
            char flag;
            unsigned int acked_seq;
            if ((flag = _read_worker(SYNC_TEMPO, (char*)&acked_seq, sizeof(int))) == ACK && acked_seq == _last_local_seq_number){ // If final ACK, or if we had lost it, data packet
                _status = ESTABLISHED;
                postOffice->registerCloseHandler(lcl_box, this);
                DEBUG('L', "Connection::Accept -- Connected\n");
                _lock->Release();
                return true;
            }
            else if (flag != TIMEOUT){
                DEBUG('L', "Attempt %d on %d failed: after acking seq is %d instead of %d\n", attempts+1, MAXREEMISSIONS, acked_seq, _last_local_seq_number); 
                _send_worker(RESET, TEMPO);
                break;
            }
            else if (++attempts == MAXREEMISSIONS || current_time > (unsigned int)timeout)
                break;
        }
        DEBUG('L', "Connection::Accept -- TIMEOUT\n");
    } while (timeout < 0 || current_time < (unsigned int)timeout);
    _lock->Release();
    
    rmt_adr = -1;
    rmt_box = -1;
    _status = IDLE;

    return false;
}

bool Connection::Connect(int timeout){
    ASSERT(_status == IDLE);
    
    unsigned int attempts = 0;
    
    _status = CONNECTING;
    _last_local_seq_number =  0;
    _last_remote_seq_number = 0;

    do {
        char flags;
        
        if (_send_worker(START, timeout) != 0) // Timeout
            continue;
        DEBUG('L', "Connection::Connect -- START\n");
        
        if ((flags = _read_worker(timeout)) < 0 || flags != (START | ACK)){ // We strongly want only a start +ack flag, otherwise we abort. We should maybe try again
            continue;
        }
        DEBUG('L', "Connection::Connect -- START | ACK\n");
        
        if (_send_worker(ACK, timeout, (char*)&_last_remote_seq_number, sizeof(int)) != sizeof(int)) // Timeout
            continue;
        DEBUG('L', "Connection::Connect -- ACK\n");
            
        postOffice->registerCloseHandler(lcl_box, this);
        _status = ESTABLISHED;
        return true;
    } while (++attempts < MAXREEMISSIONS);
    
    return false;
}

bool Connection::Close(int timeout, bool receiving){
    ASSERT(_status == ESTABLISHED);
    
    unsigned int attempts = 0;
    
    _status = CLOSING;
    DEBUG('L', "Closing connection...\n");

    do {
        char flags;
        
        DEBUG('L', "Connection::Close: %s END...\n", receiving ? "receving" : "sending");
        if (receiving ? (_read_worker(timeout) == END): (_send_worker(END, timeout) != 0)) // Timeout
            continue;
        DEBUG('L', "Connection::Close: %s END\n", receiving ? "recv" : "sent");
        
        if (!receiving){
            DEBUG('L', "Connection::Close: receving END | ACK...\n");
            if ((flags = _read_worker(timeout)) != (END | ACK)){
                if (flags ==  END && _last_local_seq_number < _last_remote_seq_number){ // peer already closing us
                    receiving = true;
                    continue;
                }
                DEBUG('L', "Receive %d\n", flags);
            }
            DEBUG('L', "Connection::Close: recv ACK|END\n");
        }
        
        DEBUG('L', "Connection::Close: sending END | ACK...\n");
        _send_worker(END | ACK, timeout);
        DEBUG('L', "Connection::Close: sent END | ACK\n");
        
        if (receiving){
            DEBUG('L', "Connection::Close: receving END | ACK...\n");
            flags = _read_worker(timeout);
            DEBUG('L', "Receive %d\n", flags);
            DEBUG('L', "Connection::Close: asuming recv ACK|END\n");
            
            DEBUG('L', "Connection::Close: sending END | ACK...\n");
            if ((flags = _send_worker(ACK, timeout)) > 0) // Timeout
                DEBUG('L', "Connection::Close: recv ACK\n");
        } else if ((flags = _read_worker(timeout)) > 0) // Timeout
            DEBUG('L', "Connection::Close: recv ACK\n");
                
        DEBUG('N', "Disconnect has got success\n");
        
        _status = CLOSED;
        
        DEBUG('L', "Connection closed\n");
        return true;
    } while (++attempts < MAXREEMISSIONS);
    
    DEBUG('L', "Connection not closed\n");
        
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
    memset(outBuffer, 0, MaxMailSize);
    memcpy(outBuffer, &outTrHdr, sizeof(TransferHeader));
    memcpy(outBuffer + sizeof(TransferHeader), data, length);

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
        return TIMEOUT;
    } else {
        memcpy(&inTrHdr, inBuffer, sizeof(TransferHeader));
        memcpy(data, inBuffer + sizeof(TransferHeader), inMailHdr.length < length ? inMailHdr.length : length);
                                                      // `-> inMailHdr.length?
        _last_remote_seq_number = inTrHdr.seq_num;
        
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
