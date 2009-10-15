#ifndef _MESSAGE_LOADBANG_H_
#define _MESSAGE_LOADBANG_H_

#include "MessageInputMessageOutputObject.h"

/**
 * loadbang
 */
class MessageLoadbang : public MessageInputMessageOutputObject {
  
  public:
    MessageLoadbang(char *initString);
    ~MessageLoadbang();
  
    void process();
  
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
  
  private:
    bool isFirstIteration;
};

#endif // _MESSAGE_LOADBANG_H_
