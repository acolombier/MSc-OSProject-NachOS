// nettest.cc
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//		-m <ID>: set the machine ID
//		-o <ID>: mail the machine identified by <ID>
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "transfer.h"

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    const char *data = "Hello there!";
    const char *ack = "Got it!";
    char buffer[MaxMailSize];

    int i;
    for (i = 0; i < 5; i++)
    {
        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = farAddr;
        outMailHdr.to = 0;
        outMailHdr.from = 1;
        outMailHdr.length = strlen(data) + 1;

        // Send the first message
        postOffice->Send(outPktHdr, outMailHdr, data);

        // Wait for the first message from the other machine
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from, inMailHdr.from);
        fflush(stdout);

        // Send acknowledgement to the other machine (using "reply to" mailbox
        // in the message that just arrived
        outPktHdr.to = inPktHdr.from;
        outMailHdr.to = inMailHdr.from;
        outMailHdr.length = strlen(ack) + 1;
        postOffice->Send(outPktHdr, outMailHdr, ack);

        // Wait for the ack from the other machine to the first message we sent.
        postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from, inMailHdr.from);

        DEBUG('n', "=============== Finished a round ===============\n");
        fflush(stdout);
    }

    // Then we're done!
    interrupt->Halt();
}

void TransferTest(int farAddr, int isSender)
{
    const char payload_0[] = "abcdefghijklmnopqrstuvwxyz";
    const char payload_1[] = "My money's in that office, right? If she start giving me some bullshit about it ain't there, and we got to go someplace else and get it, I'm gonna shoot you in the head then and there. Then I'm gonna shoot that bitch in the kneecaps, find out where my goddamn money is. She gonna tell me too. Hey, look at me when I'm talking to you, motherfucker. You listen: we go in there, and that nigga Winston or anybody else is in there, you the first motherfucker to get shot. You understand?";
    const char payload_2[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed doeiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enimad minim veniam, quis nostrud exercitation ullamco laboris nisi utaliquip ex ea commodo consequat. Duis aute irure dolor inreprehenderit in voluptate velit esse cillum dolore eu fugiat nullapariatur. Excepteur sint occaecat cupidatat non proident, sunt inculpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed doeiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enimad minim veniam, quis nostrud exercitation ullamco laboris nisi utaliquip ex ea commodo consequat. Duis aute irure dolor inreprehenderit in voluptate velit esse cillum dolore eu fugiat nullapariatur. Excepteur sint occaecat cupidatat non proident, sunt inculpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed doeiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enimad minim veniam, quis nostrud exercitation ullamco laboris nisi utaliquip ex ea commodo consequat. Duis aute irure dolor inreprehenderit in voluptate velit esse cillum dolore eu fugiat nullapariatur. Excepteur sint occaecat cupidatat non proident, sunt inculpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed doeiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enimad minim veniam, quis nostrud exercitation ull";
    const char payload_3[] = "On m'appelle l'ovniOn m'appelle l'ovniOn m'appelle l'ovniOn m'appelle l'ovniOn m'appelle l'ovniJe suis parti, je squatte plus le bétonJ'aide mes potes qui sont bés-tomJe sers, que ça retourne son vestonIls se reconnaissent dans mes sonsAlcoolisé au guidon, je fais des doigts d'honneurs aux schmitsFaut voir ce que nous vivons entre les histoires et les flûtesQue ça mitonne, oh ohMême plus je m'étonne, oh ohEt si mon heure sonne, oh ohPleure pas rigole, oh oh (l'ovni)J'ai ma team, pour ça que je m'en tape de la promo' (l'ovni)J'ai la rime, je peux y arriver, ça serait beau (l'ovni)";
    
    const char* pl_list[] = {payload_0, payload_1, payload_2, payload_3};

    if (isSender > 0) {
        Connection *conn = new Connection(postOffice->assignateBox(), farAddr, 4);
        if (conn->Connect(TEMPO)){
            printf("Server is ready\n");
            int select_data = 3;
            unsigned int size = strlen(pl_list[select_data]);
            printf("Sending the payload %d of size %d, %p\n", select_data, size, (void*)size);
            if (!conn->Send((char*)&size, sizeof(int)))
                printf("\n\tCouldn't send the size!\n\n");
            else
                printf("\n\tPayload has%s been sent!\n\n%s\n\n", (conn->Send(pl_list[select_data], size)) ? "" : " NOT", pl_list[select_data]);
        } else
            printf("Cannot connect: timeout\n");
        delete conn;
    } else {
        Connection *conn = new Connection(4);
        if (conn->Accept()){
            printf("Client is ready\n");
            unsigned int size;
            while (conn->Receive((char*)&size, sizeof(int))){
                printf("Receiving the payload of size %d, %p\n", size, (void*)size);
                char* buffer = (char*) malloc((size + 1) * sizeof(char));
                if (!conn->Receive(buffer, size))
                    break;
                buffer[size] = '\0';
                printf("Got \"%s\"\n", buffer);
            }
        }
        delete conn;
    }
    
    DEBUG('n', "=============== Finished a round ===============\n");
    fflush(stdout);
}
