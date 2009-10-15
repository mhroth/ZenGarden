#ifndef _DSP_MESSAGE_INPUT_MESSAGE_OUTPUT_OBJECT_H_
#define _DSP_MESSAGE_INPUT_MESSAGE_OUTPUT_OBJECT_H_

#include "DspMessagePresedence.h"
#include "List.h"
#include "MessageOutputObject.h"
#include "PdMessage.h"

class DspMessageInputMessageOutputObject : public MessageOutputObject {
  
public:
  DspMessageInputMessageOutputObject(int numInlets, int numOutlets, int blockSize, char *initString);
  virtual ~DspMessageInputMessageOutputObject();
  
  void addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex);
  void process();
  List *getEvaluationOrdering();
  
protected:
  inline void prepareInputBuffers();
  inline void processMessages();
  inline void resetNextMessageCounter();
  inline PdMessage *getNextMessageInTemporalOrder(int *inletIndex);
  virtual void processMessage(int inletIndex, PdMessage *pdMessage) = 0;
  virtual void processDspToIndex(int newBlockIndex) = 0;
  bool isRoot();
  
  int numInlets;
  int blockSize;
  int numBytesInBlock;
  List **incomingDspConnectionsListAtInlet;
  float **localDspBufferAtInlet;
  List **incomingMessageConnectionsListAtInlet;
  int **nextMessageCounter;
  int blockIndexOfLastMessage;
  DspMessagePresedence signalPresedence;
};

#endif // _DSP_MESSAGE_INPUT_MESSAGE_OUTPUT_OBJECT_H_
