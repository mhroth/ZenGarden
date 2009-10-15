#ifndef _MESSAGE_MODULUS_H_
#define _MESSAGE_MODULUS_H_

#include "MessageBinaryOperationObject.h"

/**
 * +
 */
class MessageModulus : public MessageBinaryOperationObject {
  
  public:
    MessageModulus(char *initString);
    MessageModulus(float constant, char *initString);
    ~MessageModulus();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_MODULUS_H_
