#ifndef _MESSAGE_UNPACK_H_
#define _MESSAGE_UNPACK_H_

#include "MessageInputMessageOutputObject.h"

/**
 * unpack
 */
class MessageUnpack : public MessageInputMessageOutputObject {
  
  public:
    MessageUnpack(List *messageElementList, char *initString);
    ~MessageUnpack();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
  
  private:
    List *messageElementList;
};

#endif // _MESSAGE_UNPACK_H_
