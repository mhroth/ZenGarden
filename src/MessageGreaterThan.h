#ifndef _MESSAGE_GREATER_THAN_H_
#define _MESSAGE_GREATER_THAN_H_

#include "MessageBinaryOperationObject.h"

/**
 * >
 */
class MessageGreaterThan : public MessageBinaryOperationObject {
  
  public:
    MessageGreaterThan(char *initString);
    MessageGreaterThan(float constant, char *initString);
    ~MessageGreaterThan();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_GREATER_THAN_H_
