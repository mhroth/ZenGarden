#ifndef _DSP_VOLTAGE_CONTROLLED_BAND_PASS_FILTER_H_
#define _DSP_VOLTAGE_CONTROLLED_BAND_PASS_FILTER_H_

#include "DspMessageInputDspOutputObject.h"

/**
 * vcf~, a voltage controlled bandpass filter
 */
class DspVCF : public DspMessageInputDspOutputObject {
  
  public:
    DspVCF(int blockSize, int sampleRate, char *initString);
    DspVCF(float q, int blockSize, int sampleRate, char *initString);
    ~DspVCF();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    void processDspToIndex(int newBlockIndex);
    
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

#endif // _DSP_VOLTAGE_CONTROLLED_BAND_PASS_FILTER_H_
