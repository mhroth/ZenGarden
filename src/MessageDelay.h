#ifndef _MESSAGE_DELAY_H_
#define _MESSAGE_DELAY_H_

#include "DspMessageInputMessageOutputObject.h"

/**
 * del
 *
 * This object doesn't really take dsp as an input, but it does have a sense of time
 * and must be able to fire events in relation to dsp.
 */
class MessageDelay : public DspMessageInputMessageOutputObject {
  
  public:
    MessageDelay(int blockSize, int sampleRate, char *initString);
    MessageDelay(float delayInMs, int blockSize, int sampleRate, char *initString);
    ~MessageDelay();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    inline void processDspToIndex(int newBlockIndex);
    
  private:
    float sampleRate;
    int delayInSamples;
    int numSamplesToDeadline;
};

#endif // _MESSAGE_DELAY_H_
