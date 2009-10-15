#ifndef _DSP_DELAY_WRITE_H_
#define _DSP_DELAY_WRITE_H_

#include "RemoteBufferObject.h"

class DspDelayWrite : public RemoteBufferObject {
  
  public:
    DspDelayWrite(float delayInMs, char *tag, int blockSize, int sampleRate, char *initString);
    ~DspDelayWrite();
  
    void processDspToIndex(int newBlockIndex);
  
  private:
    float sampleRate;
};

#endif // _DSP_DELAY_WRITE_H_
