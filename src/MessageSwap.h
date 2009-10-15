#ifndef _MESSAGE_SWAP_H_
#define _MESSAGE_SWAP_H_

#include "MessageInputMessageOutputObject.h"

class MessageSwap : public MessageInputMessageOutputObject {
  
  public:
    MessageSwap(char *initString);
    MessageSwap(float constant, char *initString);
    ~MessageSwap();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
  
  private:
    float left;
    float right;
};

#endif // _MESSAGE_SWAP_H_
