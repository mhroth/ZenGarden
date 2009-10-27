/*
 *  Copyright 2009 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 * 
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "DspLetIndex.h"
#include "DspMessageInputDspOutputObject.h"
#include "MessageLetIndex.h"
#include "MessageOutputObject.h"

DspMessageInputDspOutputObject::DspMessageInputDspOutputObject(int numInlets, int numOutlets, int blockSize, char *initString) : DspOutputObject(numOutlets, blockSize, initString) {
  this->numInlets = numInlets;
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

DspMessageInputDspOutputObject::~DspMessageInputDspOutputObject() {
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

void DspMessageInputDspOutputObject::prepareInputBuffers() {
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

void DspMessageInputDspOutputObject::processMessages() {
  resetNextMessageCounter();
  
  PdMessage *message;
  int inletIndex;
  while ((message = getNextMessageInTemporalOrder(&inletIndex)) != NULL) {
    processMessage(inletIndex, message);
  }
}

PdMessage *DspMessageInputDspOutputObject::getNextMessageInTemporalOrder(int *inletIndex) {
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

inline void DspMessageInputDspOutputObject::resetNextMessageCounter() {
  for (int i = 0; i < numInlets; i++) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[i];
    memset(nextMessageCounter[i], 0, incomingMessageConnectionsList->getNumElements() * sizeof(int));
  }
}

void DspMessageInputDspOutputObject::process() {
  blockIndexOfLastMessage = 0; // reset the block index of the last message
  prepareInputBuffers(); // sum all of the incoming DSP streams to the local inlet buffers
  processMessages(); // process all the incoming messages
  processDspToIndex(blockSize); // finish processing the local dsp
}

void DspMessageInputDspOutputObject::addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex) {
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

bool DspMessageInputDspOutputObject::isRoot() {
  for (int i = 0; i < numInlets; i++) {
    if (incomingMessageConnectionsListAtInlet[i]->getNumElements() > 0 ||
        incomingDspConnectionsListAtInlet[i]->getNumElements() > 0) {
      return false;
    }
  }
  return true;
}

List *DspMessageInputDspOutputObject::getEvaluationOrdering() {
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
