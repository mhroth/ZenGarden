#ifndef _DSP_SIG_H_
#define _DSP_SIG_H_

#include "MessageInputDspOutputObject.h"

/**
 * sig~
 */
class DspSig : public MessageInputDspOutputObject {
  
  public:
    DspSig(int blockSize, char *initString);
    DspSig(float constant, int blockSize, char *initString);
    ~DspSig();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    void processDspToIndex(int newBlockIndex);
    
  private:
    float constant;
    bool constWasReset;
    float *buffer;
};

#endif // _DSP_SIG_H_
