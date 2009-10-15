#ifndef _DSP_CLIP_H_
#define _DSP_CLIP_H_

#include "DspMessageInputDspOutputObject.h"

/**
 * clip~
 */
class DspClip : public DspMessageInputDspOutputObject {
  
  public:
    DspClip(int blockSize, char *initString);
    DspClip(float lowerBound, int blockSize, char *initString);
    DspClip(float lowerBound, float upperBound, int blockSize, char *initString);
    ~DspClip();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    void processDspToIndex(int newBlockIndex);
  
  private:
    float lowerBound;
    float upperBound;
};

#endif // _DSP_CLIP_H_
