#ifndef _MESSAGE_SUBTRACT_H_
#define _MESSAGE_SUBTRACT_H_

#include "MessageBinaryOperationObject.h"

/**
 * -
 */
class MessageSubtract : public MessageBinaryOperationObject {
  
  public:
    MessageSubtract(char *initString);
    MessageSubtract(float constant, char *initString);
    ~MessageSubtract();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_SUBTRACT_H_
