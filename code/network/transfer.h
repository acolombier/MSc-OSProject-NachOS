#ifndef TRANSFER_H
#define TRANSFER_H


#include "network.h"
#include "post.h"

#define TEMPO 1000
#define MAXREEMISSIONS 5

#define MaxMailSize (MaxWireSize - sizeof(struct PacketHeader) - sizeof(struct MailHeader))
//  TODO: not good, miss things
#define MaxMessageSize (MaxWireSize - sizeof(struct PacketHeader) - sizeof(struct TransferHeader))

// The following class defines part of the message header.
// This is prepended to the message by the RTFM, before the message
// is sent to the Network.

enum {
    ACK = 1 << 2,
    SYN = 1 << 1,
    FIN = 1 << 0
};

class TransferHeader {
  public:
    char flags;
    int seqNumber;
};

class Connection {
  public:
    Connection(NetworkAddress to, MailBoxAddress mailbox);
    ~Connection();

    int Send(char *data);  // send a message reliably with ack
    int Receive(char *data);  // receive a message reliably with ack, store it in data
    int SendFile(int fd, int fileSize);  // send a file from a client machine to remote machine
    int ReceiveFile(int fd, int fileSize);  // receive a file from a remote machine to a client machine

  private:
    NetworkAddress toMachine;	// Destination machine ID
    MailBoxAddress toMail;		// Destination mail box
    PostOffice postOffice;
    int localSeqNumber;
    int remoteSeqNumber;

    int SendFixedSize(char *data);  // send a message of size == MaxPacketSize
    int ReceiveFixedSize(char *data);  // receive a message of size == MaxPacketSize
};


#endif  /* TRANSFER_H */


/* usage example:

Connection conn = new Connection(42, 1);
conn->Send(*data);
char *data = conn->Recieve();
delete conn;
*/
