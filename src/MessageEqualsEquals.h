#ifndef _MESSAGE_EQUALS_EQUALS_H_
#define _MESSAGE_EQUALS_EQUALS_H_

#include "MessageBinaryOperationObject.h"

/**
 * ==
 */
class MessageEqualsEquals : public MessageBinaryOperationObject {
  
  public:
    MessageEqualsEquals(char *initString);
    MessageEqualsEquals(float constant, char *initString);
    ~MessageEqualsEquals();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_EQUALS_EQUALS_H_
