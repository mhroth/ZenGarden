#ifndef _MESSAGE_SPIGOT_H_
#define _MESSAGE_SPIGOT_H_

#include "MessageInputMessageOutputObject.h"

/**
 * spigot
 */
class MessageSpigot : public MessageInputMessageOutputObject {
  
  public:
    MessageSpigot(char *initString);
    MessageSpigot(float initialValue, char *initString);
    ~MessageSpigot();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
  
  private:
    float right;
};

#endif // _MESSAGE_SPIGOT_H_
