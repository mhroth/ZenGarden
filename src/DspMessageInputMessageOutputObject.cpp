#include "DspLetIndex.h"
#include "DspMessageInputMessageOutputObject.h"
#include "DspOutputObject.h"
#include "MessageLetIndex.h"
#include "MessageOutputObject.h"

DspMessageInputMessageOutputObject::DspMessageInputMessageOutputObject(int numInlets, int numOutlets, int blockSize, char *initString) : 
    MessageOutputObject(numOutlets, initString) {
  this->numInlets = numInlets;
  this->blockSize = blockSize;
  blockIndexOfLastMessage = 0;
  signalPresedence = MESSAGE_MESSAGE; // default
  numBytesInBlock = blockSize * sizeof(float);
  incomingDspConnectionsListAtInlet = (List **) malloc(numInlets * sizeof(List *));
  for (int i = 0; i < numInlets; i++) {
    incomingDspConnectionsListAtInlet[i] = new List();
  }
  localDspBufferAtInlet = (float **) malloc(numInlets * sizeof(float *));
  for (int i = 0; i < numInlets; i++) {
    localDspBufferAtInlet[i] = (float *) calloc(blockSize, sizeof(float));
  }
  incomingMessageConnectionsListAtInlet = (List **) malloc(numInlets * sizeof(List *));
  for (int i = 0; i < numInlets; i++) {
    incomingMessageConnectionsListAtInlet[i] = new List();
  }
  nextMessageCounter = (int **) calloc(numInlets, sizeof(int *));
  for (int i = 0; i < numInlets; i++) {
    nextMessageCounter[i] = (int *) calloc(8, sizeof(int));
  }
}

DspMessageInputMessageOutputObject::~DspMessageInputMessageOutputObject() {
  // free incomingDspConnectionsListAtInlet
  for (int i = 0; i < numInlets; i++) {
    List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[i];
    for (int j = 0; j < incomingDspConnectionsList->getNumElements(); j++) {
      free((DspLetIndex *) incomingDspConnectionsList->get(j));
    }
    delete incomingDspConnectionsList;
  }
  free(incomingDspConnectionsListAtInlet);
  
  // free localDspBufferAtInlet
  for (int i = 0; i < numInlets; i++) {
    free(localDspBufferAtInlet[i]);
  }
  free(localDspBufferAtInlet);
  
  // free incomingMessageConnectionsListAtInlet
  for (int i = 0; i < numInlets; i++) {
    List *list = incomingMessageConnectionsListAtInlet[i];
    for (int j = 0; j < list->getNumElements(); j++) {
      delete (MessageLetIndex *) list->get(j);
    }
  }
  free(incomingMessageConnectionsListAtInlet);
  
  for (int i = 0; i < numInlets; i++) {
    free(nextMessageCounter[i]);
  }
  free(nextMessageCounter);
}

inline void DspMessageInputMessageOutputObject::prepareInputBuffers() {
  for (int i = 0; i < numInlets; i++) {
    List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[i];
    int numElements = incomingDspConnectionsList->getNumElements();
    if (numElements > 0) {
      DspLetIndex *dspLetIndex = (DspLetIndex *) incomingDspConnectionsList->get(0);
      memcpy(localDspBufferAtInlet[i], 
             dspLetIndex->dspObject->getDspBufferAtOutlet(dspLetIndex->letIndex), 
             numBytesInBlock);
      if (numElements > 1) {
        for (int j = 1; j < numElements; j++) {
          DspLetIndex *dspLetIndex = (DspLetIndex *) incomingDspConnectionsList->get(j);
          float *remoteOutputBuffer = dspLetIndex->dspObject->getDspBufferAtOutlet(dspLetIndex->letIndex);
          float *localInputBuffer = localDspBufferAtInlet[i];
          for (int k = 0; k < blockSize; k++) {
            localInputBuffer[k] += remoteOutputBuffer[k];
          }
        }
      }
    }
  }
}

inline void DspMessageInputMessageOutputObject::processMessages() {
  resetNextMessageCounter();
  
  PdMessage *message;
  int inletIndex;
  while ((message = getNextMessageInTemporalOrder(&inletIndex)) != NULL) {
    processMessage(inletIndex, message);
  }
}

inline PdMessage *DspMessageInputMessageOutputObject::getNextMessageInTemporalOrder(int *inletIndex) {
  PdMessage *nextMessage = NULL;
  int minimumBlockIndex = 0x7FFFFFFF; // the largest possible integer
  int objectIndex = -1;
  for (int i = 0; i < numInlets; i++) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
    int numMessageConnections = incomingMessageConnectionsList->getNumElements();
    for (int j = 0; j < numMessageConnections; j++) {
      MessageLetIndex *messageLetIndex = (MessageLetIndex *) incomingMessageConnectionsList->get(j);
      PdMessage *message = messageLetIndex->messageObject->getMessageAtOutlet(
                                                                              messageLetIndex->letIndex, nextMessageCounter[i][j]);
      if (message != NULL && message->getBlockIndex() < minimumBlockIndex) {
        nextMessage = message;
        minimumBlockIndex = message->getBlockIndex();
        *inletIndex = i;
        objectIndex = j;
        if (minimumBlockIndex == 0) {
          // the minimum possible block index is zero,
          // so we won't be able to find anything that happens before
          break;
        }
      }
    }
  }
  if (nextMessage != NULL) {
    nextMessageCounter[*inletIndex][objectIndex]++;
  }
  return nextMessage;
}

inline void DspMessageInputMessageOutputObject::resetNextMessageCounter() {
  for (int i = 0; i < numInlets; i++) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
    memset(nextMessageCounter[i], 0, incomingMessageConnectionsList->getNumElements() * sizeof(int));
  }
}

void DspMessageInputMessageOutputObject::process() {
  blockIndexOfLastMessage = 0; // reset the block index of the last message
  resetOutgoingMessageBuffers(); // reset the outgoing message buffers
  prepareInputBuffers(); // sum all of the incoming DSP streams to the local inlet buffers
  processMessages(); // process all of the incoming messages
  processDspToIndex(blockSize); // finish processing the local dsp
}

void DspMessageInputMessageOutputObject::addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex) {
  switch (pdObject->getObjectType()) {
    case DSP: {
      List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[inletIndex];
      DspLetIndex *dspLetIndex = (DspLetIndex *) malloc(sizeof(DspLetIndex));
      dspLetIndex->dspObject = (DspOutputObject *) pdObject;
      dspLetIndex->letIndex = outletIndex;
      incomingDspConnectionsList->add(dspLetIndex);
      switch (signalPresedence) {
        case DSP_MESSAGE: {
          signalPresedence = (inletIndex == 0) ? DSP_MESSAGE : DSP_DSP;
          break;
        }
        case MESSAGE_DSP: {
          signalPresedence = (inletIndex == 0) ? DSP_DSP : MESSAGE_DSP;
          break;
        }
        case DSP_DSP: {
          signalPresedence = DSP_DSP;
          break;
        }
        case MESSAGE_MESSAGE: {
          signalPresedence = (inletIndex == 0) ? DSP_MESSAGE : MESSAGE_DSP;
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case MESSAGE: {
      List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[inletIndex];
      MessageLetIndex *messageLetIndex = (MessageLetIndex *) malloc(sizeof(MessageLetIndex));
      messageLetIndex->messageObject = (MessageOutputObject *) pdObject;
      messageLetIndex->letIndex = outletIndex;
      incomingMessageConnectionsList->add(messageLetIndex);
      break;
    }
    default: {
      break;
    }
  }
}

bool DspMessageInputMessageOutputObject::isRoot() {
  for (int i = 0; i < numInlets; i++) {
    if (incomingMessageConnectionsListAtInlet[i]->getNumElements() > 0 ||
        incomingDspConnectionsListAtInlet[i]->getNumElements() > 0) {
      return false;
    }
  }
  return true;
}

List *DspMessageInputMessageOutputObject::getEvaluationOrdering() {
  if (isMarkedForEvaluation) {
    return new List(); // return a list of length 0
  } else {
    isMarkedForEvaluation = true;
    if (isRoot()) {
      List *list = new List();
      list->add(this);
      return list; // return a list containing only this Node
    } else {
      List *list = new List();
      for (int i = 0; i < numInlets; i++) {
        // examine all message connections
        List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
        for (int j = 0; j < incomingMessageConnectionsList->getNumElements(); j++) {
          MessageLetIndex *messageLetIndex = (MessageLetIndex *) incomingMessageConnectionsList->get(j);
          List *parentList = messageLetIndex->messageObject->getEvaluationOrdering();
          list->add(parentList);
          delete parentList;
        }
        
        // examine all dsp connections
        List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[i];
        for (int j = 0; j < incomingDspConnectionsList->getNumElements(); j++) {
          DspLetIndex *dspLetIndex = (DspLetIndex *) incomingDspConnectionsList->get(j);
          List *parentList = dspLetIndex->dspObject->getEvaluationOrdering();
          list->add(parentList);
          delete parentList;
        }
      }
      list->add(this);
      return list;
    }
  }
}
