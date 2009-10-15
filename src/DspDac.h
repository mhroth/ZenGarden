#ifndef _DSP_DAC_H_
#define _DSP_DAC_H_

#include "DspInputDspOutputObject.h"

/**
 * dac~
 */
class DspDac : public DspInputDspOutputObject {
  
  public:
    DspDac(int blockSize, char *initString);
    ~DspDac();
  
    void processDspToIndex(int newBlockIndex);
};

#endif // _DSP_DAC_H_
