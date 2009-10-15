#ifndef _MESSAGE_INLET_OUTLET_H_
#define _MESSAGE_INLET_OUTLET_H_

#include "MessageInputMessageOutputObject.h"

class MessageInletOutlet : public MessageInputMessageOutputObject {
  
  public:
    MessageInletOutlet(char *initString);
    virtual ~MessageInletOutlet();
  
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
};

#endif // _MESSAGE_INLET_OUTLET_H_
