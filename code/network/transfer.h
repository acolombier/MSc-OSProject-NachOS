#ifndef TRANSFER_H
#define TRANSFER_H


#include "network.h"
#include "post.h"

#define TEMPO 100000000
#define MAXREEMISSIONS 5

#define MAX_MESSAGE_SIZE (MaxWireSize - sizeof(struct PacketHeader) - sizeof(struct MailHeader) - sizeof(struct TransferHeader))

// The following class defines part of the message header.
// This is prepended to the message by the RTFM, before the message
// is sent to the Network.sizeof(struct PacketHeader)
class TransferHeader {
  public:
    TransferHeader(char _flags = 0, unsigned int _seq_num = 0):
        flags(_flags), seq_num(_seq_num){}
    char flags;
    unsigned int seq_num;
};

class Connection {
  public:
    enum Flag {
        ACK     = 0b100,
        START   = 0b010,
        END     = 0b001
    };
    
    enum Status {
        IDLE,
        ACCEPTING,
        CONNECTING,
        ESTABLISHED,
        CLOSING,
        CLOSED
    };

    Connection(MailBoxAddress localbox, NetworkAddress to = -1, MailBoxAddress mailbox = -1, Status s = IDLE);
    ~Connection();

    /*!
     * \todo doc
     */
    bool Send(char *data, size_t length);
    
    /*!
     * \todo doc
     */
    bool Receive(char *data, size_t length);
    
    /*!
     *  \brief Turn the socket in to a server socket, and wait a client to connect
     */
    Connection* Accept(int timeout = -1);
    
    /*!
     *  \brief Turn the socket in to a client socket, and try to connect the remote peer
     */
    bool Connect(int timeout = -1);
    
    /*!
     *  \brief This method initialise the last handshake of the syncronisation. It should be called only by the `Accept` factory.
     */
    bool Synch(int timeout);
    
    inline Status status() const { return _status; }
    
    /*!
     *  \brief Turn the socket in to a client socket, and try to connect the remote peer
     */
    bool Close(int timeout = -1);

  private:
    Status _status;
    unsigned int _last_local_seq_number;
    unsigned int _last_remote_seq_number;
    
    MailBoxAddress lcl_box;
    NetworkAddress rmt_adr;	// Destination machine ID
    MailBoxAddress rmt_box;		// Destination mail box
    

    int _send_worker(char flags, int timeout, char* data = nullptr, size_t length = 0);  // send a message of size == MaxPacketSize
    char _read_worker(int timeout, char* data = nullptr, size_t length = 0, NetworkAddress*a = nullptr, MailBoxAddress*p = nullptr);  // receive a message of size == MaxPacketSize
    
    char *flagstostr(char flags);
};


#endif  /* TRANSFER_H */
