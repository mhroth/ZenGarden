#ifndef _MESSAGE_ADD_H_
#define _MESSAGE_ADD_H_

#include "MessageBinaryOperationObject.h"

/**
 * +
 */
class MessageAdd : public MessageBinaryOperationObject {
  
  public:
    MessageAdd(char *initString);
    MessageAdd(float constant, char *initString);
    ~MessageAdd();
  
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_ADD_H_
