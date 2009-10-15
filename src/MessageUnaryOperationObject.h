#ifndef _MESSAGE_UNARY_OPERATION_OBJECT_H_
#define _MESSAGE_UNARY_OPERATION_OBJECT_H_

#include "MessageInputMessageOutputObject.h"

class MessageUnaryOperationObject : public MessageInputMessageOutputObject {
  
  public:
    MessageUnaryOperationObject(char *initString);
    virtual ~MessageUnaryOperationObject();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    virtual float performUnaryOperation(float input) = 0;
};

#endif // _MESSAGE_UNARY_OPERATION_OBJECT_H_
