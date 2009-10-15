#ifndef _DSP_PHASOR_H_
#define _DSP_PHASOR_H_

#include "DspMessageInputDspOutputObject.h"

/**
 * phasor~
 */
class DspPhasor : public DspMessageInputDspOutputObject {
  
  public:
    DspPhasor(int blockSize, int sampleRate, char *initString); // and oscillator of default zero frequency
    DspPhasor(float frequency, int blockSize, int sampleRate, char *initString); // for adding a constant to a dsp stream
    ~DspPhasor();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    inline void processDspToIndex(int newBlockIndex);
    
  private:    
    float sampleRate;
    float frequency; // frequency and phase are stored as integers because they are used
    float phase;
    float slope;
    float lastOutput;
};

#endif // _DSP_PHASOR_H_
