#ifndef TRANSFER_H
#define TRANSFER_H


#include "network.h"
#include "post.h"

#define TEMPO 1000
#define MAXREEMISSIONS 5

class RTFM {
  public:
    RTFM(to, mailbox);
    ~RTFM();

    int Send(char *data, FixedHeader header); //
    int Recive(PacketHeader *pktHdr, MailHeader *mailHdr, char *data);
};


#endif  /* TRANSFER_H */

/* usage example:

RTFM conn = new RTFM(42, 1);
conn->Send(*data);
char *data = conn->Recieve();
delete RTFM();
*/
