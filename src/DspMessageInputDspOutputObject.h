#ifndef _DSP_MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_
#define _DSP_MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_

#include "DspMessagePresedence.h"
#include "DspOutputObject.h"
#include "List.h"
#include "PdMessage.h"

class DspMessageInputDspOutputObject : public DspOutputObject {
  
  public:
    DspMessageInputDspOutputObject(int numInlets, int numOutlets, int blockSize, char *initString);
    virtual ~DspMessageInputDspOutputObject();
    
    void addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex);
    void process();
    List *getEvaluationOrdering();
    
  protected:
    void prepareInputBuffers();
    void processMessages();
    void resetNextMessageCounter();
    PdMessage *getNextMessageInTemporalOrder(int *inletIndex);
    virtual void processMessage(int inletIndex, PdMessage *pdMessage) = 0;
    virtual void processDspToIndex(int newBlockIndex) = 0;
    bool isRoot();
    
    int numInlets;
    int numBytesInBlock;
    List **incomingDspConnectionsListAtInlet;
    float **localDspBufferAtInlet;
    List **incomingMessageConnectionsListAtInlet;
    int **nextMessageCounter;
    int blockIndexOfLastMessage;
    DspMessagePresedence signalPresedence;
};

#endif // _DSP_MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_
