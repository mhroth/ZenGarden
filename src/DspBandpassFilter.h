#ifndef _DSP_BAND_PASS_FILTER_H_
#define _DSP_BAND_PASS_FILTER_H_

#include "DspMessageInputDspOutputObject.h"

/**
 * Implements the bp~ (2-pole) dsp object.
 */
class DspBandpassFilter : public DspMessageInputDspOutputObject {
  
  public:
    DspBandpassFilter(int blockSize, int sampleRate, char *initString);
    DspBandpassFilter(float centerFrequency, float q, int blockSize, int sampleRate, char *initString);
    ~DspBandpassFilter();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    inline void processDspToIndex(int newBlockIndex);
    
  private:
    void calculateFilterCoefficients(float f, float q);
    float sigbp_qcos(float f); // not entirely sure what this is doing. From Pd.
  
    float sampleRate;
    float centerFrequency;
    float q;
    float coef1;
    float coef2;
    float gain;
    float tap_0;
    float tap_1;
};

#endif // _DSP_BAND_PASS_FILTER_H_
