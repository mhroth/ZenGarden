#ifndef _DSP_INPUT_DSP_OUTPUT_OBJECT_H_
#define _DSP_INPUT_DSP_OUTPUT_OBJECT_H_

#include "DspOutputObject.h"
#include "List.h"

class DspInputDspOutputObject : public DspOutputObject {
  
  public:
    DspInputDspOutputObject(int numInlets, int numOutlets, int blockSize, char *initString);
    virtual ~DspInputDspOutputObject();
    
    float *getDspBufferAtInlet(int inletIndex);
    
    List *getEvaluationOrdering();
    void process();
    void addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex);
    
  protected:
    /**
     * Sums all incoming audio buffers to the local input buffers
     */
    void prepareInputBuffers();
    bool isRoot();
    virtual void processDspToIndex(int newBlockIndex) = 0;
    
    int numInlets;
    int blockSize;
    int numBytesInBlock;
    List **incomingDspConnectionsListAtInlet;
    float **localDspBufferAtInlet;
};

#endif // _DSP_INPUT_DSP_OUTPUT_OBJECT_H_
