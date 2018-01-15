#ifndef TRANSFER_H
#define TRANSFER_H


#include "network.h"
#include "post.h"

#define TEMPO 1000
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
    char flags;
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

    int SendFixedSize(char *data, char flags);  // send a message of size == MaxPacketSize
    char ReceiveFixedSize(char *data);  // receive a message of size == MaxPacketSize
    char *flagstostr(char flags);
};


#endif  /* TRANSFER_H */


/* usage example:

Connection conn = new Connection(42, 1);
conn->Send(*data);
char *data = conn->Recieve();
delete conn;
*/
