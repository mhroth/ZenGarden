#ifndef _DSP_LOW_PASS_FILTER_H_
#define _DSP_LOW_PASS_FILTER_H_

#include "DspMessageInputDspOutputObject.h"

/**
 * lop~
 * Specficially implement a one-tap IIR filter: y = alpha * x_0 + (1-alpha) * x_-1
 */
class DspLowpassFilter : public DspMessageInputDspOutputObject {
  
  public:
    DspLowpassFilter(int blockSize, int sampleRate, char *iniString);
    DspLowpassFilter(float cutoffFrequency, int blockSize, int sampleRate, char *iniString);
    ~DspLowpassFilter();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    inline void processDspToIndex(int newBlockIndex);
    
  private:
    void calculateFilterCoefficients(float cutoffFrequency);
  
    float sampleRate;
    float tap_0;
    float alpha;
    float beta; // 1 - alpha;
};

#endif // _DSP_LOW_PASS_FILTER_H_
