#ifndef _MESSAGE_RMS_TO_DB_H_
#define _MESSAGE_RMS_TO_DB_H_

#include "MessageUnaryOperationObject.h"

class MessageRmsToDb : public MessageUnaryOperationObject {
  
  public:
    MessageRmsToDb(char *initString);
    ~MessageRmsToDb();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_RMS_TO_DB_H_
