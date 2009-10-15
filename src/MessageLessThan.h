#ifndef _MESSAGE_LESS_THAN_H_
#define _MESSAGE_LESS_THAN_H_

#include "MessageBinaryOperationObject.h"

/**
 * <
 */
class MessageLessThan : public MessageBinaryOperationObject {
  
  public:
    MessageLessThan(char *initString);
    MessageLessThan(float constant, char *initString);
    ~MessageLessThan();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_LESS_THAN_H_
