#ifndef _MESSAGE_BINARY_OPERATION_OBJECT_H_
#define _MESSAGE_BINARY_OPERATION_OBJECT_H_

#include "MessageInputMessageOutputObject.h"

class MessageBinaryOperationObject : public MessageInputMessageOutputObject {
  
  public:
    MessageBinaryOperationObject(char *initString);
    virtual ~MessageBinaryOperationObject();
  
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    virtual float performBinaryOperation(float left, float right) = 0;
  
    float left;
    float right;
};

#endif // _MESSAGE_BINARY_OPERATION_OBJECT_H_
