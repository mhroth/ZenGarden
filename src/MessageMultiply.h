#ifndef _MESSAGE_MULTIPLY_H_
#define _MESSAGE_MULTIPLY_H_

#include "MessageBinaryOperationObject.h"

/**
 * *
 */
class MessageMultiply : public MessageBinaryOperationObject {
  
  public:
    MessageMultiply(char *initString);
    MessageMultiply(float constant, char *initString);
    ~MessageMultiply();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_MULTIPLY_H_
