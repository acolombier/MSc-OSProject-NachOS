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

enum {
    ACK = 1 << 2,
    START = 1 << 1,
    END = 1 << 0
};

class TransferHeader {
  public:
    unsigned int seq_num;
    char flags;
};

class Connection {
  public:
    Connection(MailBoxAddress localbox, NetworkAddress to, MailBoxAddress mailbox);
    ~Connection();

    int Send(char *data);  // send a message reliably with ack
    void Receive(char *data);  // receive a message reliably with ack, store it in data
    int SendFile(int fd, int fileSize);  // send a file from a client machine to remote machine
    int ReceiveFile(int fd, int fileSize);  // receive a file from a remote machine to a client machine

  private:
    MailBoxAddress lcl_box;
    NetworkAddress rmt_adr;	// Destination machine ID
    MailBoxAddress rmt_box;		// Destination mail box

    int SendFixedSize(char *data, size_t length, unsigned int seq_num, char flags);  // send a message of size == MaxPacketSize
    void ReceiveFixedSize(char *data);  // receive a message of size == MaxPacketSize
    char *flagstostr(char flags);
};


#endif  /* TRANSFER_H */


/* usage example:

Connection conn = new Connection(42, 1);
conn->Send(*data);
char *data = conn->Recieve();
delete conn;
*/
