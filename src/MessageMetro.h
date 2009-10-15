#ifndef _MESSAGE_METRO_H_
#define _MESSAGE_METRO_H_

#include "MessageInputMessageOutputObject.h"

/**
 * metro
 */
class MessageMetro : public MessageInputMessageOutputObject {
  
  public:
    MessageMetro(int blockSize, int sampleRate, char *initString);
    MessageMetro(float intervalInMs, int blockSize, int sampleRate, char *initString);
    ~MessageMetro();
    
  protected:
    void process();
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    bool isOn;
    int intervalInSamples;
    int index;
    int blockSize;
    float sampleRate;
};

#endif // _MESSAGE_METRO_H_
