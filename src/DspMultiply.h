#ifndef _DSP_MULTIPLY_H_
#define _DSP_MULTIPLY_H_

#include "DspBinaryOperationObject.h"

/**
 * *~
 */
class DspMultiply : public DspBinaryOperationObject {
  
  public:
    DspMultiply(int blockSize, char *initString);
    DspMultiply(float constant, int blockSize, char *initString);
    ~DspMultiply();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _DSP_MULTIPLY_H_
