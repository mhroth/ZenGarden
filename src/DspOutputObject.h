#ifndef _DSP_OUTPUT_OBJECT_H_
#define _DSP_OUTPUT_OBJECT_H_

#include "PdObject.h"

class DspOutputObject : public PdObject {
  
  public:
    DspOutputObject(int numOutlets, int blockSize, char *initString);
    virtual ~DspOutputObject();
    
    PdObjectType getObjectType();
  
    float *getDspBufferAtOutlet(int outletIndex);
    
  protected:
    int numOutlets;
    int blockSize;
    int numBytesInBlock;
    float **localDspBufferAtOutlet;
};

#endif // _DSP_OUTPUT_OBJECT_H_
