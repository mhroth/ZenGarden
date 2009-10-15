#ifndef _MESSAGE_MOSES_H_
#define _MESSAGE_MOSES_H_

#include "MessageInputMessageOutputObject.h"

/**
 * moses
 */
class MessageMoses : public MessageInputMessageOutputObject {
  
  public:
    MessageMoses(char *initString);
    MessageMoses(float threshold, char *initString);
    ~MessageMoses();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    float threshold;
};

#endif // _MESSAGE_MOSES_H_
