#ifndef _DSP_NOISE_H_
#define _DSP_NOISE_H_

#include "DspInputDspOutputObject.h"

/**
 * noise~
 */
class DspNoise : public DspInputDspOutputObject {
    
  public:
    DspNoise(int blockSize, char *initString);
    ~DspNoise();
    
  protected:
    void processDspToIndex(int newBlockIndex);
  
  private:
    const static float floatHalfRandMax;
};

#endif // _DSP_NOISE_H_
