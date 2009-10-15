#ifndef _MESSAGE_COSINE_H_
#define _MESSAGE_COSINE_H_

#include "MessageUnaryOperationObject.h"

class MessageCosine: public MessageUnaryOperationObject {
  
  public:
    MessageCosine(char *initString);
    ~MessageCosine();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_COSINE_H_
