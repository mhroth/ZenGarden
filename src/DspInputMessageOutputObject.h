#ifndef _DSP_INPUT_MESSAGE_OUTPUT_OBJECT_H_
#define _DSP_INPUT_MESSAGE_OUTPUT_OBJECT_H_

#include "MessageOutputObject.h"

class DspInputMessageOutputObject : public MessageOutputObject {
  
  public:
    DspInputMessageOutputObject(int numInlets, int numOutlets, int blockSize, char *iniString);
    ~DspInputMessageOutputObject();
    
    float *getDspBufferAtInlet(int inletIndex);
    
    List *getEvaluationOrdering();
    void process();
    virtual void processDspToIndex(int newBlockIndex) = 0;
    void addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex);
    
  protected:
    bool isRoot();
    void prepareInputBuffers();
    
    int numInlets;
    int blockSize;
    int numBytesInBlock;
    List **incomingDspConnectionsListAtInlet;
    float **localDspBufferAtInlet;
};

#endif // _DSP_INPUT_MESSAGE_OUTPUT_OBJECT_H_
