#ifndef _MESSAGE_SQRT_H_
#define _MESSAGE_SQRT_H_

#include "MessageUnaryOperationObject.h"

class MessageSqrt : public MessageUnaryOperationObject {
  
  public:
    MessageSqrt(char *initString);
    ~MessageSqrt();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_SQRT_H_
