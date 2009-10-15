#ifndef _MESSAGE_TRIGGER_H_
#define _MESSAGE_TRIGGER_H_

#include "MessageInputMessageOutputObject.h"

/**
 * Implements the trigger/t message object.
 * This object basically casts the input to each of its output types.
 */
class MessageTrigger : public MessageInputMessageOutputObject {
  
  public:
    MessageTrigger(List *messageElementList, char *initString);
    ~MessageTrigger();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
  
  private:
    int numCasts;
    MessageElementType *castArray;
};

#endif // _MESSAGE_TRIGGER_H_
