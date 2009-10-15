#ifndef _MESSAGE_FLOAT_H_
#define _MESSAGE_FLOAT_H_

#include "MessageInputMessageOutputObject.h"

class MessageFloat : public MessageInputMessageOutputObject {
  
  public:
    MessageFloat(char *initString);
    MessageFloat(float newConstant, char *initString);
    ~MessageFloat();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:  
    float constant;
};

#endif // _MESSAGE_FLOAT_H_
