#ifndef _MESSAGE_ABSOLUTE_VALUE_H_
#define _MESSAGE_ABSOLUTE_VALUE_H_

#include "MessageInputMessageOutputObject.h"

/**
 * abs
 */
class MessageAbsoluteValue : public MessageInputMessageOutputObject {
    
  public:
    MessageAbsoluteValue(char *initString);
    ~MessageAbsoluteValue();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
};

#endif // _MESSAGE_ABSOLUTE_VALUE_H_
