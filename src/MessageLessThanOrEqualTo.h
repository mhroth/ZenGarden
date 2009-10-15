#ifndef _MESSAGE_LESS_THAN_OR_EQUAL_TO_H_
#define _MESSAGE_LESS_THAN_OR_EQUAL_TO_H_

#include "MessageBinaryOperationObject.h"

/**
 * <=
 */
class MessageLessThanOrEqualTo : public MessageBinaryOperationObject {
  
  public:
    MessageLessThanOrEqualTo(char *initString);
    MessageLessThanOrEqualTo(float constant, char *initString);
    ~MessageLessThanOrEqualTo();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_LESS_THAN_OR_EQUAL_TO_H_
