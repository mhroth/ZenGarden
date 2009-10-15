#include "MessageInputDspOutputObject.h"
#include "MessageLetIndex.h"
#include "MessageOutputObject.h"

MessageInputDspOutputObject::MessageInputDspOutputObject(int numInlets, int numOutlets, int blockSize, char *initString) : DspOutputObject(numOutlets, blockSize, initString) {
  this->numInlets = numInlets;
  blockIndexOfLastMessage = 0;
  incomingMessageConnectionsListAtInlet = (List **) malloc(numInlets * sizeof(List *));
  for (int i = 0; i < numInlets; i++) {
    incomingMessageConnectionsListAtInlet[i] = new List();
  }
  nextMessageCounter = (int **) calloc(numInlets, sizeof(int *));
  // WARNING: setting the maximum number of message connections per inlet to 8 is a complete hack
  // this is done because for some strange reason, the machine didn't like it when the length of
  // the arrays were adjusted when connections are made
  for (int i = 0; i < numInlets; i++) {
    nextMessageCounter[i] = (int *) calloc(8, sizeof(int));
  }
}

MessageInputDspOutputObject::~MessageInputDspOutputObject() {
  for (int i = 0; i < numInlets; i++) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
    for (int j = 0; j < incomingMessageConnectionsList->getNumElements(); j++) {
      free((MessageLetIndex *) incomingMessageConnectionsList->get(j));
    }
    delete incomingMessageConnectionsList;
  }
  free(incomingMessageConnectionsListAtInlet);
  
  for (int i = 0; i < numInlets; i++) {
    free(nextMessageCounter[i]);
  }
  free(nextMessageCounter);
}

void MessageInputDspOutputObject::processMessages() {
  resetNextMessageCounter();
  
  PdMessage *message;
  int inletIndex;
  while ((message = getNextMessageInTemporalOrder(&inletIndex)) != NULL) {
    processMessage(inletIndex, message);
  }
}

PdMessage *MessageInputDspOutputObject::getNextMessageInTemporalOrder(int *inletIndex) {
  PdMessage *nextMessage = NULL;
  int minimumBlockIndex = 0x7FFFFFFF; // the largest possible integer
  int objectIndex = -1;
  for (int i = 0; i < numInlets; i++) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
    for (int j = 0; j < incomingMessageConnectionsList->getNumElements(); j++) {
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

void MessageInputDspOutputObject::resetNextMessageCounter() {
  for (int i = 0; i < numInlets; i++) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
    memset(nextMessageCounter[i], 0, incomingMessageConnectionsList->getNumElements() * sizeof(int));
  }
}

void MessageInputDspOutputObject::process() {
  blockIndexOfLastMessage = 0;
  processMessages(); // process incoming messages
  processDspToIndex(blockSize);
}

bool MessageInputDspOutputObject::isRoot() {
  for (int i = 0; i < numInlets; i++) {
    if (incomingMessageConnectionsListAtInlet[i]->getNumElements() > 0) {
      return false;
    }
  }
  return true;
}

void MessageInputDspOutputObject::addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex) {
  if (pdObject->getObjectType() == MESSAGE) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[inletIndex];
    MessageLetIndex *messageLetIndex = (MessageLetIndex *) malloc(sizeof(MessageLetIndex));
    messageLetIndex->messageObject = (MessageOutputObject *) pdObject;
    messageLetIndex->letIndex = outletIndex;
    incomingMessageConnectionsList->add(messageLetIndex);
  }
}

List *MessageInputDspOutputObject::getEvaluationOrdering() {
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
      }
      list->add(this);
      return list;
    }
  }
}
