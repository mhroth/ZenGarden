#ifndef _MESSAGE_CHANGE_H_
#define _MESSAGE_CHANGE_H_

#include "MessageInputMessageOutputObject.h"

/**
 * change
 */
class MessageChange : public MessageInputMessageOutputObject {
  
  public:
    MessageChange(char *initString);
    MessageChange(float initialValue, char *initString);
    ~MessageChange();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    float left;
};

#endif // _MESSAGE_CHANGE_H_
