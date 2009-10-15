#ifndef _MESSAGE_DB_TO_POW_H_
#define _MESSAGE_DB_TO_POW_H_

#include "MessageUnaryOperationObject.h"

class MessageDbToPow : public MessageUnaryOperationObject {
  
  public:
    MessageDbToPow(char *initString);
    ~MessageDbToPow();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_DB_TO_POW_H_
