/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#include "ArrayArithmetic.h"
#include "DspObject.h"
#include "PdGraph.h"

float *DspObject::zeroBuffer = NULL;
int DspObject::zeroBufferCount = 0;
int DspObject::zeroBufferSize = 0;

DspObject::DspObject(int numMessageInlets, int numDspInlets, int numMessageOutlets, int numDspOutlets, PdGraph *graph) :
    MessageObject(numMessageInlets, numMessageOutlets, graph) {
  init(numDspInlets, numDspOutlets, graph->getBlockSize());
}

DspObject::DspObject(int numMessageInlets, int numDspInlets, int numMessageOutlets, int numDspOutlets, int blockSize, PdGraph *graph) : 
    MessageObject(numMessageInlets, numMessageOutlets, graph) {
  init(numDspInlets, numDspOutlets, blockSize);
}

void DspObject::init(int numDspInlets, int numDspOutlets, int blockSize) {
  this->numDspInlets = numDspInlets;
  this->numDspOutlets = numDspOutlets;
  blockSizeInt = blockSize;
  blockSizeFloat = (float) blockSizeInt;
  blockIndexOfLastMessage = 0.0f;
  signalPrecedence = MESSAGE_MESSAGE; // default
  numBytesInBlock = blockSizeInt * sizeof(float);
  dspBufferRefAtInlet0 = NULL;
  dspBufferRefAtInlet1 = NULL;
  numConnectionsToInlet0 = 0;
  numConnectionsToInlet1 = 0;
  
  if (zeroBufferSize < blockSize) {
    zeroBuffer = (float *) realloc(zeroBuffer, blockSize * sizeof(float));
    memset(zeroBuffer, 0, blockSize * sizeof(float));
    zeroBufferSize = blockSize;
  }
  zeroBufferCount++;
  
  // initialise the incoming dsp connections list
  incomingDspConnectionsListAtInlet =
      (numDspInlets > 0) ? (vector<ObjectLetPair> **) malloc(numDspInlets * sizeof(vector<ObjectLetPair> *)) : NULL;
  for (int i = 0; i < numDspInlets; i++) {
    incomingDspConnectionsListAtInlet[i] = new vector<ObjectLetPair>;
  }
  
  // initialise the outgoing dsp connections list
  outgoingDspConnectionsListAtOutlet =
      (numDspOutlets > 0) ? (vector<ObjectLetPair> **) malloc(numDspOutlets * sizeof(vector<ObjectLetPair> *)) : NULL;
  for (int i = 0; i < numDspOutlets; i++) {
    outgoingDspConnectionsListAtOutlet[i] = new vector<ObjectLetPair>;
  }
  
  dspBufferAtInlet = (numDspInlets > 2) ? (float **) calloc(numDspInlets, sizeof(float *)) : NULL;
  
  if (numDspInlets > 0) {
    dspBufferRefListAtInlet = (List **) malloc(numDspInlets * sizeof(List *));
    for (int i = 0; i < numDspInlets; i++) {
      dspBufferRefListAtInlet[i] = new List();
    }
  } else {
    dspBufferRefListAtInlet = NULL;
  }
  
  // initialise the local output audio buffers
  if (numDspOutlets > 0) {
    localDspOutletBuffers = (float *) calloc(numDspOutlets * blockSize, sizeof(float));
    dspBufferAtOutlet0 = localDspOutletBuffers;
  } else {
    localDspOutletBuffers = NULL;
    dspBufferAtOutlet0 = NULL;
  }
  if (numDspOutlets > 1) {
    dspBufferAtOutlet = (float **) calloc(numDspOutlets, sizeof(float *));
    for (int i = 1; i < numDspOutlets; i++) {
      dspBufferAtOutlet[i] = localDspOutletBuffers + (i*blockSizeInt);
    }
  } else {
    dspBufferAtOutlet = NULL;
  }
}

DspObject::~DspObject() {  
  if (--zeroBufferCount == 0) {
    free(zeroBuffer);
    zeroBuffer = NULL;
    zeroBufferSize = 0;
  }
  
  // free the incoming dsp connections list
  for (int i = 0; i < numDspInlets; i++) {
    delete incomingDspConnectionsListAtInlet[i];
  }
  free(incomingDspConnectionsListAtInlet);
  
  // free the outgoing dsp connections list
  for (int i = 0; i < numDspOutlets; i++) {
    delete outgoingDspConnectionsListAtOutlet[i];
  }
  free(outgoingDspConnectionsListAtOutlet);
  
  for (int i = 0; i < numDspInlets; i++) {
    List *dspBufferRefList = (List *) dspBufferRefListAtInlet[i];
    delete dspBufferRefList;
  }
  free(dspBufferRefListAtInlet);
  
  // free the local output audio buffers
  free(localDspOutletBuffers);
  free(dspBufferAtInlet);
  free(dspBufferAtOutlet);
}

ConnectionType DspObject::getConnectionType(int outletIndex) {
  return DSP;
}

float **DspObject::getDspBufferRefAtOutlet(int outletIndex) {
  if (outletIndex == 0) {
    return &dspBufferAtOutlet0;
  } else {
    return dspBufferAtOutlet + outletIndex;
  }
}

bool DspObject::doesProcessAudio() {
  return true;
}

void DspObject::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  MessageObject::addConnectionFromObjectToInlet(messageObject, outletIndex, inletIndex);
  
  if (messageObject->getConnectionType(outletIndex) == DSP) {
    vector<ObjectLetPair> *connections = incomingDspConnectionsListAtInlet[inletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, outletIndex);
    connections->push_back(objectLetPair);
    
    // set signal precedence
    signalPrecedence = (DspMessagePresedence) (signalPrecedence | (0x1 << inletIndex));
    
    DspObject *dspObject = (DspObject *) messageObject;
    List *dspBufferRefList = dspBufferRefListAtInlet[inletIndex];
    dspBufferRefList->add(dspObject->getDspBufferRefAtOutlet(outletIndex));
    if (inletIndex == 0) {
      if (++numConnectionsToInlet0 == 1) {
        dspBufferRefAtInlet0 = (float **) dspBufferRefList->get(0);
      }
    } else if (inletIndex == 1) {
      if (++numConnectionsToInlet1 == 1) {
        dspBufferRefAtInlet1 = (float **) dspBufferRefList->get(0);
      }
    }
  }
}

void DspObject::addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  MessageObject::addConnectionToObjectFromOutlet(messageObject, inletIndex, outletIndex);
  
  // TODO(mhroth): it is assumed here that the input connection type of the destination object is DSP. Correct?
  if (getConnectionType(outletIndex) == DSP) {
    vector<ObjectLetPair> *connections = outgoingDspConnectionsListAtOutlet[outletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, inletIndex);
    connections->push_back(objectLetPair);
  }
}

bool DspObject::shouldDistributeMessageToInlets() {
  return false;
}

void DspObject::receiveMessage(int inletIndex, PdMessage *message) {
  // Queue the message to be processed during the DSP round only if the graph is switched on.
  // Otherwise messages would begin to pile up because the graph is not processed.
  if (graph->isSwitchedOn()) {
    // Copy the message to the heap so that it is available to process later.
    // The message is released once it is consumed in processDsp().
    messageQueue.push(make_pair(message->copyToHeap(), inletIndex));
  }
}

void DspObject::processDsp() {
  switch (numDspInlets) {
    default: {
      List *dspBufferRefList;
      for (int i = 2; i < numDspInlets; i++) {
        dspBufferRefList = dspBufferRefListAtInlet[i];
        switch (dspBufferRefList->numElements) {
          case 0: {
            dspBufferAtInlet[i] = zeroBuffer;
            break;
          }
          case 1: {
            float **bufferRef = (float **) dspBufferRefList->arrayList[0];
            dspBufferAtInlet[i] = *bufferRef;
            break;
          }
          default: {
            dspBufferAtInlet[i] = (float *) alloca(numBytesInBlock);
            resolveInputBuffers(i, dspBufferAtInlet[i]);
            break;
          }
        }
      }
      // allow fallthrough
    }
    case 2: {
      switch (numConnectionsToInlet1) {
        case 0: {
          dspBufferAtInlet1 = zeroBuffer;
          break;
        }
        case 1: {
          dspBufferAtInlet1 = *dspBufferRefAtInlet1;
          break;
        }
        default: { // > 1
          // allocate a temporary buffer on the stack. Large buffer sizes could cause a stack overflow
          dspBufferAtInlet1 = (float *) alloca(numBytesInBlock);
          resolveInputBuffers(1, dspBufferAtInlet1);
          break;
        }
      }
      // allow fallthrough
    }
    case 1: {
      switch (numConnectionsToInlet0) {
        case 0: {
          dspBufferAtInlet0 = zeroBuffer;
          break;
        }
        case 1: {
          dspBufferAtInlet0 = *dspBufferRefAtInlet0;
          break;
        }
        default: { // > 1
          // allocate a temporary buffer on the stack. Large buffer sizes could cause a stack overflow
          dspBufferAtInlet0 = (float *) alloca(numBytesInBlock);
          resolveInputBuffers(0, dspBufferAtInlet0);
          break;
        }
      }
      // allow fallthrough
    }
    case 0: {
      break; // nothing to do
    }
  }
  
  // process all pending messages in this block
  if (messageQueue.empty()) {
    processDspWithIndex(0, blockSizeInt);
  } else {
    do { // there is at least one message
      MessageLetPair messageLetPair = messageQueue.front();
      PdMessage *message = messageLetPair.first;
      unsigned int inletIndex = messageLetPair.second;
      
      processMessage(inletIndex, message);
      message->freeMessage(); // free the message from the head, the message has been consumed.
      messageQueue.pop();
      
      blockIndexOfLastMessage = graph->getBlockIndex(message);
    } while (!messageQueue.empty());
    processDspWithIndex(blockIndexOfLastMessage, blockSizeFloat);
    blockIndexOfLastMessage = 0.0f; // reset the block index of the last received message
  }
}

void DspObject::resolveInputBuffers(int inletIndex, float *localInputBuffer) {
  List *dspBufferRefList = dspBufferRefListAtInlet[inletIndex];
  
  // copy the single connection's output buffer to the input buffer
  int numConnections = dspBufferRefList->numElements;
  float ***refArray = (float ***) dspBufferRefList->arrayList;
  memcpy(localInputBuffer, *(refArray[0]), numBytesInBlock);

  
  // add the remaining output buffers to the input buffer
  for (int j = 1; j < numConnections; j++) {
    ArrayArithmetic::add(localInputBuffer, *(refArray[j]), localInputBuffer, 0, blockSizeInt);
  }
}

void DspObject::processDspWithIndex(float fromIndex, float toIndex) {
  // by default, this function just calls the integer version with adjusted block indicies
  processDspWithIndex((int) ceilf(fromIndex), (int) ceilf(toIndex));
}

void DspObject::processDspWithIndex(int fromIndex, int toIndex) {
  // by default, this function just calls the float version
  processDspWithIndex((float) fromIndex, (float) toIndex);
}

bool DspObject::isLeafNode() {
  if (!MessageObject::isLeafNode()) {
    return false;
  } else {
    for (int i = 0; i < numDspOutlets; i++) {
      if (outgoingDspConnectionsListAtOutlet[i]->size() > 0) {
        return false;
      }
    }
    return true;
  }
}

List *DspObject::getProcessOrder() {
  if (isOrdered) {
    // if this object has already been ordered, then move on
    return new List();
  } else {
    isOrdered = true;
    List *processList = new List();
    for (int i = 0; i < numMessageInlets; i++) {
      for (int j = 0; j < incomingMessageConnectionsListAtInlet[i]->size(); j++) {
        ObjectLetPair objectLetPair = incomingMessageConnectionsListAtInlet[i]->at(j);
        List *parentProcessList = objectLetPair.first->getProcessOrder();
        processList->add(parentProcessList);
        delete parentProcessList;
      }
    }
    for (int i = 0; i < numDspInlets; i++) {
      for (int j = 0; j < incomingDspConnectionsListAtInlet[i]->size(); j++) {
        ObjectLetPair objectLetPair = incomingDspConnectionsListAtInlet[i]->at(j);
        List *parentProcessList = objectLetPair.first->getProcessOrder();
        processList->add(parentProcessList);
        delete parentProcessList;
      }
    }
    processList->add(this);
    return processList;
  }
}
