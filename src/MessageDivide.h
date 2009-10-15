#ifndef _MESSAGE_DIVIDE_H_
#define _MESSAGE_DIVIDE_H_

#include "MessageBinaryOperationObject.h"

/**
 * /
 */
class MessageDivide : public MessageBinaryOperationObject {
  
  public:
    MessageDivide(char *initString);
    MessageDivide(float constant, char *initString);
    ~MessageDivide();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_DIVIDE_H_
