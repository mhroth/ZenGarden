#ifndef _MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_
#define _MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_

#include "DspOutputObject.h"
#include "List.h"
#include "PdMessage.h"

class MessageInputDspOutputObject : public DspOutputObject {
  
  public:
    MessageInputDspOutputObject(int numInlets, int numOutlets, int blockSize, char *initString);
    virtual ~MessageInputDspOutputObject();
    
    void addConnectionFromObjectToInlet(PdObject *object, int outletIndex, int inletIndex);
    void process();
    List *getEvaluationOrdering();
    
  protected:
    PdMessage *getNextMessageInTemporalOrder(int *inletIndex);
    void resetNextMessageCounter();
    void processMessages();
    virtual void processMessage(int inletIndex, PdMessage *message) = 0;
    bool isRoot();
    virtual void processDspToIndex(int newBlockIndex) = 0;
    
    int numInlets;
    List **incomingMessageConnectionsListAtInlet;
    int **nextMessageCounter;
    int blockIndexOfLastMessage;
};

#endif // _MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_
