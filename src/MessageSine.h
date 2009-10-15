#ifndef _MESSAGE_SINE_H_
#define _MESSAGE_SINE_H_

#include "MessageUnaryOperationObject.h"

class MessageSine : public MessageUnaryOperationObject {
  
  public:
    MessageSine(char *initString);
    ~MessageSine();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_SINE_H_
