#ifndef _MESSAGE_INPUT_MESSAGE_OUTPUT_OBJECT_H_
#define _MESSAGE_INPUT_MESSAGE_OUTPUT_OBJECT_H_

#include "MessageLetIndex.h"
#include "MessageOutputObject.h"

class MessageInputMessageOutputObject : public MessageOutputObject {
  
  public:
    MessageInputMessageOutputObject(int numInlets, int numOutlets, char *initString);
    virtual ~MessageInputMessageOutputObject();
    
    void addConnectionFromObjectToInlet(PdObject *object, int outletIndex, int inletIndex);
    void process();
    List *getEvaluationOrdering();
  
  protected:
    PdMessage *getNextMessageInTemporalOrder(int *inletIndex);
    void resetNextMessageCounter();
    void processMessages();
    virtual void processMessage(int inletIndex, PdMessage *message) = 0;
    bool isRoot();
  
    int numInlets;
    List **incomingMessageConnectionsListAtInlet;
    int **nextMessageCounter;
};

#endif // _MESSAGE_INPUT_MESSAGE_OUTPUT_OBJECT_H_
