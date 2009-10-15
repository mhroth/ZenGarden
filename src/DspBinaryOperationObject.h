#ifndef _DSP_BINARY_OPERATION_OBJECT_H_
#define _DSP_BINARY_OPERATION_OBJECT_H_

#include "DspMessageInputDspOutputObject.h"

class DspBinaryOperationObject : public DspMessageInputDspOutputObject {
  
public:
  DspBinaryOperationObject(int blockSize, char *initString);
  virtual ~DspBinaryOperationObject();
  
protected:
  void processMessage(int inletIndex, PdMessage *message);
  void processDspToIndex(int newBlockIndex);
  virtual float performBinaryOperation(float left, float right) = 0;
  
  float constant;
};

#endif // _DSP_BINARY_OPERATION_OBJECT_H_
