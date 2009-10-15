#ifndef _DSP_LINE_H_
#define _DSP_LINE_H_

#include "MessageInputDspOutputObject.h"

/**
 * line~
 */
class DspLine : public MessageInputDspOutputObject {
  
  public:
    DspLine(int blockSize, int sampleRate, char *initString);
    ~DspLine();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    inline void processDspToIndex(int newBlockIndex);
    
  private:
    float sampleRate;
    float target;
    float slope;
    int numSamplesToTarget;
    float lastOutputSample;
};

#endif // _DSP_LINE_H_
