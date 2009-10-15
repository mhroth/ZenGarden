#ifndef _MESSAGE_DB_TO_RMS_H_
#define _MESSAGE_DB_TO_RMS_H_

#include "MessageUnaryOperationObject.h"

class MessageDbToRms : public MessageUnaryOperationObject {
  
  public:
    MessageDbToRms(char *initString);
    ~MessageDbToRms();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_DB_TO_RMS_H_
