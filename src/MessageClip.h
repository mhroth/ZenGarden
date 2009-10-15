#ifndef _MESSAGE_CLIP_H_
#define _MESSAGE_CLIP_H_

#include "MessageInputMessageOutputObject.h"

class MessageClip : public MessageInputMessageOutputObject {
  
  public:
    MessageClip(char *initString);
    MessageClip(float lowerBound, char *initString);
    MessageClip(float lowerBound, float upperBound, char *initString);
    ~MessageClip();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    float lowerBound;
    float upperBound;
};

#endif // _MESSAGE_CLIP_H_
