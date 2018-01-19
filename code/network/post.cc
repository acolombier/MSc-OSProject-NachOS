// post.cc
// 	Routines to deliver incoming network messages to the correct
//	"address" -- a mailbox, or a holding area for incoming messages.
//	This module operates just like the US postal service (in other
//	words, it works, but it's slow, and you can't really be sure if
//	your mail really got through!).
//
//	Note that once we prepend the MailHdr to the outgoing message data,
//	the combination (MailHdr plus data) looks like "data" to the Network
//	device.
//
// 	The implementation synchronizes incoming messages with threads
//	waiting for those messages.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "post.h"
#include "thread.h"
#include "transfer.h"

#include <strings.h> /* for memset */

//----------------------------------------------------------------------
// Mail::Mail
//      Initialize a single mail message, by concatenating the headers to
//	the data.
//
//	"pktH" -- source, destination machine ID's
//	"mailH" -- source, destination mailbox ID's
//	"data" -- payload data
//----------------------------------------------------------------------

Mail::Mail(PacketHeader pktH, MailHeader mailH, char *msgData)
{
    ASSERT(mailH.length <= MaxMailSize);

    pktHdr = pktH;
    mailHdr = mailH;
    bcopy(msgData, data, mailHdr.length);
}

//----------------------------------------------------------------------
// MailBox::MailBox
//      Initialize a single mail box within the post office, so that it
//	can receive incoming messages.
//
//	Just initialize a list of messages, representing the mailbox.
//----------------------------------------------------------------------


MailBox::MailBox()
{
    messages = new SynchList();
}

//----------------------------------------------------------------------
// MailBox::~MailBox
//      De-allocate a single mail box within the post office.
//
//	Just delete the mailbox, and throw away all the queued messages
//	in the mailbox.
//----------------------------------------------------------------------

MailBox::~MailBox()
{
    delete messages;
}

//----------------------------------------------------------------------
// PrintHeader
// 	Print the message header -- the destination machine ID and mailbox
//	#, source machine ID and mailbox #, and message length.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//----------------------------------------------------------------------

static void
PrintHeader(PacketHeader pktHdr, MailHeader mailHdr)
{
    fprintf(stderr, "From (%d, %d) to (%d, %d) bytes %d\n",
    	    pktHdr.from, mailHdr.from, pktHdr.to, mailHdr.to, mailHdr.length);
}

//----------------------------------------------------------------------
// MailBox::Put
// 	Add a message to the mailbox.  If anyone is waiting for message
//	arrival, wake them up!
//
//	We need to reconstruct the Mail message (by concatenating the headers
//	to the data), to simplify queueing the message on the SynchList.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//	"data" -- payload message data
//----------------------------------------------------------------------

void
MailBox::Put(PacketHeader pktHdr, MailHeader mailHdr, char *data)
{
    Mail *mail = new Mail(pktHdr, mailHdr, data);

    messages->Append((void *)mail);	// put on the end of the list of
					// arrived messages, and wake up
					// any waiters
}

//----------------------------------------------------------------------
// MailBox::Get
// 	Get a message from a mailbox, parsing it into the packet header,
//	mailbox header, and data.
//
//	The calling thread waits if there are no messages in the mailbox.
//
//	"pktHdr" -- address to put: source, destination machine ID's
//	"mailHdr" -- address to put: source, destination mailbox ID's
//	"data" -- address to put: payload message data
//----------------------------------------------------------------------

bool
MailBox::Get(PacketHeader *pktHdr, MailHeader *mailHdr, char *data, int timeout)
{
    DEBUG('N', "MailBox::Get -- Receiving....\n");
    Mail *mail = (Mail *) messages->Remove(timeout);	// remove message from list;
						// will wait if list is empty

    if (mail != NULL) {
        *pktHdr = mail->pktHdr;
        *mailHdr = mail->mailHdr;
        bcopy(mail->data, data, mail->mailHdr.length);
                        // copy the message data into
                        // the caller's buffer
        delete mail;	// we've copied out the stuff we
                        // need, we can now discard the message
        return true;
    } else 
        return false;
}

//----------------------------------------------------------------------
// PostalHelper, ReadAvail, WriteDone
// 	Dummy functions because C++ can't indirectly invoke member functions
//	The first is forked as part of the "postal worker thread; the
//	later two are called by the network interrupt handler.
//
//	"arg" -- pointer to the Post Office managing the Network
//----------------------------------------------------------------------

static void PostalHelper(int arg)
{ PostOffice* po = (PostOffice *) arg; po->PostalDelivery(); }
static void ReadAvail(int arg)
{ PostOffice* po = (PostOffice *) arg; po->IncomingPacket(); }
static void WriteDone(int arg)
{ PostOffice* po = (PostOffice *) arg; po->PacketSent(); }

//----------------------------------------------------------------------
// PostOffice::PostOffice
// 	Initialize a post office as a collection of mailboxes.
//	Also initialize the network device, to allow post offices
//	on different machines to deliver messages to one another.
//
//      We use a separate thread "the postal worker" to wait for messages
//	to arrive, and deliver them to the correct mailbox.  Note that
//	delivering messages to the mailboxes can't be done directly
//	by the interrupt handlers, because it requires a Lock.
//
//	"addr" is this machine's network ID
//	"reliability" is the probability that a network packet will
//	  be delivered (e.g., reliability = 1 means the network never
//	  drops any packets; reliability = 0 means the network never
//	  delivers any packets)
//	"nBoxes" is the number of mail boxes in this Post Office
//----------------------------------------------------------------------

PostOffice::PostOffice(NetworkAddress addr, double reliability, int nBoxes):
     _last_item_cnt(0), _availableBoxes(new BitMap(nBoxes)), closeHandler(new Connection*[nBoxes])
{
// First, initialize the synchronization with the interrupt handlers
    messageAvailable = new Semaphore("message available", 0);
    messageSent = new Semaphore("message sent", 0);
    sendLock = new Lock("message send lock");
    
    memset(closeHandler, 0, sizeof(Connection*) * nBoxes);

// Second, initialize the mailboxes
    netAddr = addr;
    numBoxes = nBoxes;
    boxes = new MailBox[nBoxes];

// Third, initialize the network; tell it which interrupt handlers to call
    network = new Network(addr, reliability, ReadAvail, WriteDone, (int) this);


// Finally, create a thread whose sole job is to wait for incoming messages,
//   and put them in the right mailbox.
    Thread *t = new Thread("postal worker");

    t->Fork(PostalHelper, (int) this);
}

//----------------------------------------------------------------------
// PostOffice::~PostOffice
// 	De-allocate the post office data structures.
//----------------------------------------------------------------------

PostOffice::~PostOffice()
{
    //~ for (int i = 0; i < numBoxes; i++)
        //~ if (closeHandler[i])
            //~ delete closeHandler[i];
    //~ delete network;
    delete [] boxes;
    delete messageAvailable;
    delete messageSent;
    delete sendLock;
    delete [] closeHandler;
    delete _availableBoxes;
}

//----------------------------------------------------------------------
// PostOffice::PostalDelivery
// 	Wait for incoming messages, and put them in the right mailbox.
//
//      Incoming messages have had the PacketHeader stripped off,
//	but the MailHeader is still tacked on the front of the data.
//----------------------------------------------------------------------

void
PostOffice::PostalDelivery()
{
    PacketHeader pktHdr;
    MailHeader mailHdr;
    char *buffer = new char[MaxPacketSize];

    for (;;) {
        // first, wait for a message
        messageAvailable->P();
        pktHdr = network->Receive(buffer);
        
        mailHdr = *(MailHeader *)buffer;

        if (DebugIsEnabled('n')) {
			fprintf(stderr, "Putting mail into mailbox: ");
			PrintHeader(pktHdr, mailHdr);
		}

		// check that arriving message is legal!
		ASSERT(0 <= mailHdr.to && mailHdr.to < numBoxes);
		ASSERT(mailHdr.length <= MaxMailSize);

		// put into mailbox
        
        if (mailHdr.to >= 0 && mailHdr.to < numBoxes && closeHandler[mailHdr.to]){ 
            TransferHeader inTrHdr;
            memcpy(&inTrHdr, buffer + sizeof(MailHeader), sizeof(TransferHeader));
            if (inTrHdr.flags == Connection::END && closeHandler[mailHdr.to]->status() == Connection::ESTABLISHED){
                while (boxes[mailHdr.to].size())
                    currentThread->Yield();
                boxes[mailHdr.to].Put(pktHdr, mailHdr, buffer + sizeof(MailHeader));
                DEBUG('L', "PostOffice::PostalDelivery -- Connection remotely closed on %d\n", mailHdr.to);
                closeHandler[mailHdr.to]->Close(TEMPO, true);
                continue;
            }
                
        }
        boxes[mailHdr.to].Put(pktHdr, mailHdr, buffer + sizeof(MailHeader));
    }
}

//----------------------------------------------------------------------
// PostOffice::Send
// 	Concatenate the MailHeader to the front of the data, and pass
//	the result to the Network for delivery to the destination machine.
//
//	Note that the MailHeader + data looks just like normal payload
//	data to the Network.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//	"data" -- payload message data
//----------------------------------------------------------------------

bool
PostOffice::Send(PacketHeader pktHdr, MailHeader mailHdr, const char* data, int timeout)
{
    char* buffer = new char[MaxPacketSize];	// space to hold concatenated
						// mailHdr + data

    ASSERT(mailHdr.length <= MaxMailSize);
    ASSERT(0 <= mailHdr.to && mailHdr.to < numBoxes);

    // fill in pktHdr, for the Network layer
    pktHdr.from = netAddr;
    pktHdr.length = mailHdr.length + sizeof(MailHeader);

    //~ if (DebugIsEnabled('n')) {
		//~ fprintf(stderr, "Post send: ");
		//~ PrintHeader(pktHdr, mailHdr);
    //~ }

    // concatenate MailHeader and data
    memcpy(buffer, &mailHdr, sizeof(MailHeader));
    memcpy(buffer + sizeof(MailHeader), data, mailHdr.length);

    sendLock->Acquire();   		// only one message can be sent
					// to the network at any one time
    network->Send(pktHdr, buffer);
    
    messageSent->P();	// wait until list isn't empty
					// ok to send the next message
    sendLock->Release();

    delete [] buffer;			// we've sent the message, so
    return _sending_msg == 0;
					// we can delete our buffer
}

//----------------------------------------------------------------------
// PostOffice::Receive
// 	Retrieve a message from a specific box if one is available,
//	otherwise wait for a message to arrive in the box.
//
//	Note that the MailHeader + data looks just like normal payload
//	data to the Network.
//
//
//	"box" -- mailbox ID in which to look for message
//	"pktHdr" -- address to put: source, destination machine ID's
//	"mailHdr" -- address to put: source, destination mailbox ID's
//	"data" -- address to put: payload message data
//----------------------------------------------------------------------

bool
PostOffice::Receive(int box, PacketHeader *pktHdr,
				MailHeader *mailHdr, char* data, int timeout)
{
    ASSERT((box >= 0) && (box < numBoxes));
    
    bool result = boxes[box].Get(pktHdr, mailHdr, data, timeout);
    
    ASSERT(mailHdr->length <= MaxMailSize);
    
    return result;
}

//----------------------------------------------------------------------
// PostOffice::IncomingPacket
// 	Interrupt handler, called when a packet arrives from the network.
//
//	Signal the PostalDelivery routine that it is time to get to work!
//----------------------------------------------------------------------

void
PostOffice::IncomingPacket()
{
    messageAvailable->V();
}

//----------------------------------------------------------------------
// PostOffice::PacketSent
// 	Interrupt handler, called when the next packet can be put onto the
//	network.
//
//	The name of this routine is a misnomer; if "reliability < 1",
//	the packet could have been dropped by the network, so it won't get
//	through.
//----------------------------------------------------------------------

void
PostOffice::PacketSent()
{
    messageSent->V(); 
}
 
MailBoxAddress PostOffice::assignateBox(){
    return _availableBoxes->Find();
}
 
bool PostOffice::acquireBox(MailBoxAddress b){
    if (!_availableBoxes->Test(b)){
        _availableBoxes->Mark(b);
        return true;
    }
    return false;
}

void PostOffice::releaseBox(MailBoxAddress b){
    _availableBoxes->Clear(b);    
    registerCloseHandler(b, nullptr);
}

void PostOffice::registerCloseHandler(MailBoxAddress box, Connection* conn){
    closeHandler[box] = conn;
}
