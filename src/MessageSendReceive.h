#ifndef _MESSAGE_SEND_RECEIVE_H_
#define _MESSAGE_SEND_RECEIVE_H_

#include "MessageInletOutlet.h"

class MessageSendReceive : public MessageInletOutlet {
  
  public:
    MessageSendReceive(char *tag, char *initString);
    virtual ~MessageSendReceive();
  
    char *getTag();
    
  protected:
    char *tag;
};

#endif // _MESSAGE_SEND_RECEIVE_H_
