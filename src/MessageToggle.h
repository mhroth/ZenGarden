#ifndef _MESSAGE_TOGGLE_H_
#define _MESSAGE_TOGGLE_H_

#include "MessageInputMessageOutputObject.h"

/**
 * toggle/tgl
 */
class MessageToggle : public MessageInputMessageOutputObject {
  
  public:
    MessageToggle(char *initString);
    ~MessageToggle();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    bool isOn;
    float constant;
};

#endif // _MESSAGE_TOGGLE_H_
