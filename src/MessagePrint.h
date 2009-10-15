#ifndef _MESSAGE_PRINT_H_
#define _MESSAGE_PRINT_H_

#include "MessageInputMessageOutputObject.h"

/**
 *  print
 *  This class does absolutely nothing :) Don't expect it to print anything.
 */
class MessagePrint : public MessageInputMessageOutputObject {
  
  public:
    MessagePrint(char *initString);
    ~MessagePrint();
  
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
};

#endif // _MESSAGE_PRINT_H_
