#ifndef _MESSAGE_POW_H_
#define _MESSAGE_POW_H_

#include "MessageBinaryOperationObject.h"

/**
 * pow
 * left^right
 */
class MessagePow : public MessageBinaryOperationObject {
  
  public:
    MessagePow(char *initString);
    MessagePow(float constant, char *initString);
    ~MessagePow();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_POW_H_
