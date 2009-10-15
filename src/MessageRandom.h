#ifndef _MESSAGE_RANDOM_H_
#define _MESSAGE_RANDOM_H_

#include "MessageInputMessageOutputObject.h"

/**
 * random
 */
class MessageRandom : public MessageInputMessageOutputObject {
  
  public:
    MessageRandom(char *initString);
    MessageRandom(int N, char *initString);
    ~MessageRandom();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    int N;
};

#endif // _MESSAGE_RANDOM_H_
