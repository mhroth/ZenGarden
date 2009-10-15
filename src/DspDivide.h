#ifndef _DSP_DIVIDE_H_
#define _DSP_DIVIDE_H_

#include "DspBinaryOperationObject.h"

/**
 * /~
 */
class DspDivide : public DspBinaryOperationObject {
  
  public:
    DspDivide(int blockSize, char *initString);
    DspDivide(float constant, int blockSize, char *initString);
    ~DspDivide();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _DSP_ADD_H_
