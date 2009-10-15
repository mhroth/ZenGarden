#ifndef _MESSAGE_EXP_H_
#define _MESSAGE_EXP_H_

#include "MessageUnaryOperationObject.h"

class MessageExp : public MessageUnaryOperationObject {
  
  public:
    MessageExp(char *initString);
    ~MessageExp();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_EXP_H_
