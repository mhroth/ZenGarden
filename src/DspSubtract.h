#ifndef _DSP_SUBTRACT_H_
#define _DSP_SUBTRACT_H_

#include "DspBinaryOperationObject.h"

/**
 * *~
 */
class DspSubtract : public DspBinaryOperationObject {
  
  public:
    DspSubtract(int blockSize, char *initString);
    DspSubtract(float constant, int blockSize, char *initString);
    ~DspSubtract();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _DSP_SUBTRACT_H_
