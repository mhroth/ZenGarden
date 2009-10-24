#ifndef _DSP_DELAY_READ_H_
#define _DSP_DELAY_READ_H_

#include "RemoteBufferReceiverObject.h"

/**
 * delread~
 */
class DspDelayRead : public RemoteBufferReceiverObject {
  
  public:
    DspDelayRead(float delayInMs, char *tag, int blockSize, int sampleRate, char *initString);
    ~DspDelayRead();
  
    void processMessage(int inletIndex, PdMessage *message);
    void processDspToIndex(int newBlockIndex);
    
  private:
    float sampleRate;
    int delayInSamples;
};

#endif // _DSP_DELAY_READ_H_
