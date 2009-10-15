#ifndef _MESSAGE_EXTERNAL_SEND_H_
#define _MESSAGE_EXTERNAL_SEND_H_

#include "MessageSendReceive.h"

/**
 *  A MessageExternalSend is generally equivalent to a MessageSendReceive
 *  but exists for the explicit purpose of bringing messages from outside
 *  of the graph (such as sensor data) into it. Only one message per interation
 *  can be accepted. This is an arbitrary limitation and serves only to simplify
 *  the implementation.
 */
class MessageExternalSend : public MessageSendReceive {
  
public:
  MessageExternalSend(char *tag, char *initString);
  ~MessageExternalSend();
  
  void process();
  
  void setExternalMessage(PdMessage *message);
  
  private:
    bool hasMessage;
    PdMessage *externalMessage;
};

#endif // _MESSAGE_EXTERNAL_SEND_H_
