#ifndef _MESSAGE_BANG_H_
#define _MESSAGE_BANG_H_

#include "MessageInputMessageOutputObject.h"

/**
 * bang, bng
 */
class MessageBang : public MessageInputMessageOutputObject {
  
  public:
    MessageBang(char *initString);
    ~MessageBang();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
};

#endif // _MESSAGE_BANG_H_
