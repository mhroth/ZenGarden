#ifndef _MESSAGE_POW_TO_DB_H_
#define _MESSAGE_POW_TO_DB_H_

#include "MessageUnaryOperationObject.h"

class MessagePowToDb : public MessageUnaryOperationObject {
  
  public:
    MessagePowToDb(char *initString);
    ~MessagePowToDb();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_POW_TO_DB_H_
