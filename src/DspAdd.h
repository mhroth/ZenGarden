#ifndef _DSP_ADD_H_
#define _DSP_ADD_H_

#include "DspBinaryOperationObject.h"

/**
 * +~
 */
class DspAdd : public DspBinaryOperationObject {
  
  public:
    DspAdd(int blockSize, char *initString);
    DspAdd(float constant, int blockSize, char *initString);
    ~DspAdd();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _DSP_ADD_H_
