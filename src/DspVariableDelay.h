#ifndef _DSP_VARIABLE_DELAY_H_
#define _DSP_VARIABLE_DELAY_H_

#include "RemoteBufferReceiverObject.h"

class DspVariableDelay : public RemoteBufferReceiverObject {
  
  public:
    DspVariableDelay(char *tag, int blockSize, int sampleRate, char *initString);
    ~DspVariableDelay();
    
    void processMessage(int inletIndex, PdMessage *message);
    void processDspToIndex(int newBlockIndex);
    
  private:
    float sampleRate;
};

#endif // _DSP_VARIABLE_DELAY_H_
