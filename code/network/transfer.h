#ifndef TRANSFER_H
#define TRANSFER_H


#include "network.h"
#include "post.h"

#define TEMPO 1000
#define MAXREEMISSIONS 5

#define MaxPacketSize 	(MaxWireSize - sizeof(struct PacketHeader) - sizeof(struct ConnectionHeader))	

// The following class defines part of the message header.  
// This is prepended to the message by the RTFM, before the message 
// is sent to the Network.

class TransferHeader {
  public:
    MailBoxAddress to;		// Destination mail box
    MailBoxAddress from;	// Mail box to reply to
    unsigned length;		// Bytes of message data (excluding the 
				// transfer header)
	int ackReceivedFrom;	// When using Send, we want ack from the address we sent message to
	int ackSentTo;			// When using Receive, we want to send ack to the address we received message from
};

class RTFM {
	public:
		RTFM(to, mailbox);
		~RTFM();

		int Send(char *data, FixedHeader header); // send a message reliably with ack
		int Receive(char *data);	// receive a message reliably with ack, store it in data
		int SendFile(int fd, int fileSize);	// send a file from a client machine to remote machine
		int ReceiveFile(int fd, int fileSize);	// receive a file from a remote machine to a client machine

	private:
		int SendFixedSize(char *data);	// send a message of size <= MaxPacketSize
		int sendVariableSize(char *data);	//	send a message of size > MaxPacketSize
		int ReceiveFixedSize(char *data);	// receive a message of size <= MaxPacketSize
		int ReceiveVariableSize(char *data);	//	receive a message of size > MaxPacketSize
		
	
};

#endif  /* TRANSFER_H */

/* usage example:

RTFM conn = new RTFM(42, 1);
conn->Send(*data);
char *data = conn->Recieve();
delete RTFM();
*/
