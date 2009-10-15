#include "MessageInputMessageOutputObject.h"

MessageInputMessageOutputObject::MessageInputMessageOutputObject(int numInlets, int numOutlets, char *initString) : MessageOutputObject(numOutlets, initString) {
  this->numInlets = numInlets;
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

MessageInputMessageOutputObject::~MessageInputMessageOutputObject() {
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

void MessageInputMessageOutputObject::processMessages() {
  resetNextMessageCounter();
  
  PdMessage *message;
  int inletIndex;
  while ((message = getNextMessageInTemporalOrder(&inletIndex)) != NULL) {
    processMessage(inletIndex, message);
  }
}

PdMessage *MessageInputMessageOutputObject::getNextMessageInTemporalOrder(int *inletIndex) {
  PdMessage *nextMessage = NULL;
  float minimumBlockIndex = HUGE_VALF;
  int objectIndex = -1;
  for (int i = 0; i < numInlets; i++) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
    for (int j = 0; j < incomingMessageConnectionsList->getNumElements(); j++) {
      MessageLetIndex *messageLetIndex = (MessageLetIndex *) incomingMessageConnectionsList->get(j);
      PdMessage *message = messageLetIndex->messageObject->getMessageAtOutlet(
          messageLetIndex->letIndex, nextMessageCounter[i][j]);
      if (message != NULL && message->getBlockIndexAsFloat() < minimumBlockIndex) {
        nextMessage = message;
        minimumBlockIndex = message->getBlockIndexAsFloat();
        *inletIndex = i;
        objectIndex = j;
        if (minimumBlockIndex == 0.0f) {
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

void MessageInputMessageOutputObject::resetNextMessageCounter() {
  // this loop is function is called very often. It can be optimised to some extent
  // by directly implementing common cases.
  switch (numInlets) {
    case 0: {
      break; // nothing to do
    }
    case 2: {
      memset(nextMessageCounter[1], 0, incomingMessageConnectionsListAtInlet[1]->getNumElements() * sizeof(int));
      // allow fallthrough
    }
    case 1: {
      memset(nextMessageCounter[0], 0, incomingMessageConnectionsListAtInlet[0]->getNumElements() * sizeof(int));
      break;
    }
    default: {
      for (int i = 0; i < numInlets; i++) {
        List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
        memset(nextMessageCounter[i], 0, incomingMessageConnectionsList->getNumElements() * sizeof(int));
      }
      break;
    }
  }
}

void MessageInputMessageOutputObject::process() {
  resetOutgoingMessageBuffers(); // for outgoing messages
  processMessages(); // process incoming messages
}

bool MessageInputMessageOutputObject::isRoot() {
  for (int i = 0; i < numInlets; i++) {
    if (incomingMessageConnectionsListAtInlet[i]->getNumElements() > 0) {
      return false;
    }
  }
  return true;
}

void MessageInputMessageOutputObject::addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex) {
  if (pdObject->getObjectType() == MESSAGE) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[inletIndex];
    MessageLetIndex *messageLetIndex = (MessageLetIndex *) malloc(sizeof(MessageLetIndex));
    messageLetIndex->messageObject = (MessageOutputObject *) pdObject;
    messageLetIndex->letIndex = outletIndex;
    incomingMessageConnectionsList->add(messageLetIndex);
  }
}

List *MessageInputMessageOutputObject::getEvaluationOrdering() {
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
