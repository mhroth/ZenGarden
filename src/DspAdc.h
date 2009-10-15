#ifndef _DSP_ADC_H_
#define _DSP_ADC_H_

#include "DspInputDspOutputObject.h"

/**
 * adc~
 */
class DspAdc : public DspInputDspOutputObject {
  
  public:
    DspAdc(int blockSize, char *initString);
    ~DspAdc();
  
    void processDspToIndex(int newBlockIndex);
  
    void copyIntoDspBufferAtOutlet(int outletIndex, float *buffer);
};

#endif // _DSP_ADC_H_
