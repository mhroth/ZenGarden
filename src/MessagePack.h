#ifndef _MESSAGE_PACK_H_
#define _MESSAGE_PACK_H_

#include "MessageInputMessageOutputObject.h"

/**
 * pack
 */
class MessagePack : public MessageInputMessageOutputObject {
  
  public:
    MessagePack(List *messageElementList, char *initString);
    ~MessagePack();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    List *messageElementList;
};

#endif // _MESSAGE_PACK_H_
