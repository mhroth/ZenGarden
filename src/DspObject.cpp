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
  messageQueue = new MessageQueue();
  
  // initialise the incoming dsp connections list
  incomingDspConnectionsListAtInlet = (List **) malloc(numDspInlets * sizeof(List *));
  for (int i = 0; i < numDspInlets; i++) {
    incomingDspConnectionsListAtInlet[i] = new List();
  }
  
  // initialise the outgoing dsp connections list
  outgoingDspConnectionsListAtOutlet = (List **) malloc(numDspOutlets * sizeof(List *));
  for (int i = 0; i < numDspOutlets; i++) {
    outgoingDspConnectionsListAtOutlet[i] = new List();
  }
  
  // initialise the local input audio buffers
  localDspBufferAtInlet = (float **) malloc(numDspInlets * sizeof(float *));
  localDspBufferAtInletReserved = (float **) malloc(numDspInlets * sizeof(float *));
  for (int i = 0; i < numDspInlets; i++) {
    localDspBufferAtInletReserved[i] = (float *) calloc(blockSizeInt, sizeof(float));
    localDspBufferAtInlet[i] = localDspBufferAtInletReserved[i];
  }
  
  // initialise the local output audio buffers
  localDspBufferAtOutlet = (float **) malloc(numDspOutlets * sizeof(float *));
  for (int i = 0; i < numDspOutlets; i++) {
    localDspBufferAtOutlet[i] = (float *) calloc(blockSizeInt, sizeof(float));
  }
}

DspObject::~DspObject() {
  delete messageQueue;
  
  // free the incoming dsp connections list
  for (int i = 0; i < numDspInlets; i++) {
    List *list = incomingDspConnectionsListAtInlet[i];
    for (int j = 0; j < list->size(); j++) {
      free(list->get(j));
    }
    delete list;
  }
  free(incomingDspConnectionsListAtInlet);
  
  // free the outgoing dsp connections list
  for (int i = 0; i < numDspOutlets; i++) {
    List *list = outgoingDspConnectionsListAtOutlet[i];
    for (int j = 0; j < list->size(); j++) {
      free(list->get(j));
    }
    delete list;
  }
  free(outgoingDspConnectionsListAtOutlet);
  
  // free the local input audio buffers
  for (int i = 0; i < numDspInlets; i++) {
    free(localDspBufferAtInletReserved[i]);
  }
  free(localDspBufferAtInlet);
  free(localDspBufferAtInletReserved);
  
  // free the local output audio buffers
  for (int i = 0; i < numDspOutlets; i++) {
    free(localDspBufferAtOutlet[i]);
  }
  free(localDspBufferAtOutlet);
}

ConnectionType DspObject::getConnectionType(int outletIndex) {
  return DSP;
}

bool DspObject::doesProcessAudio() {
  return true;
}

void DspObject::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  MessageObject::addConnectionFromObjectToInlet(messageObject, outletIndex, inletIndex);
  
  if (messageObject->getConnectionType(outletIndex) == DSP) {
    List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[inletIndex];
    ObjectLetPair *objectLetPair = (ObjectLetPair *) malloc(sizeof(ObjectLetPair));
    objectLetPair->object = messageObject;
    objectLetPair->index = outletIndex;
    incomingDspConnectionsList->add(objectLetPair);
    
    // set signal precedence
    signalPrecedence = (DspMessagePresedence) (signalPrecedence | (0x1 << inletIndex));
  }
}

void DspObject::addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  MessageObject::addConnectionToObjectFromOutlet(messageObject, inletIndex, outletIndex);
  
  // TODO(mhroth): it is assumed here that the input connection type of the destination object is DSP. Correct?
  if (getConnectionType(outletIndex) == DSP) {
    List *outgoingDspConnectionsList = outgoingDspConnectionsListAtOutlet[outletIndex];
    ObjectLetPair *objectLetPair = (ObjectLetPair *) malloc(sizeof(ObjectLetPair));
    objectLetPair->object = messageObject;
    objectLetPair->index = inletIndex;
    outgoingDspConnectionsList->add(objectLetPair);
  }
}

bool DspObject::shouldDistributeMessageToInlets() {
  return false;
}

void DspObject::receiveMessage(int inletIndex, PdMessage *message) {
  // Queue the message to be processed during the DSP round only if the graph is switched on.
  // Otherwise messages would begin to pile up because the graph is not processed.
  if (graph->isSwitchedOn()) {
    // reserve the message so that it won't be reused by the issuing object.
    // The message is released once it is consumed in processDsp().
    message->reserve(this);
    messageQueue->add(inletIndex, message);
  }
}

void DspObject::processDsp() {
  switch (numDspInlets) {
    default: {
      for (int i = 2; i < numDspInlets; i++) {
        resolveInputBuffersAtInlet(i);
      }
      // allow fallthrough
    }
    case 2: {
      resolveInputBuffersAtInlet(1);
      // allow fallthrough
    }
    case 1: {
      resolveInputBuffersAtInlet(0);
      // allow fallthrough
    }
    case 0: {
      break; // nothing to do
    }
  }
  
  // process all pending messages in this block
  MessageLetPair *messageLetPair = NULL;
  while ((messageLetPair = (MessageLetPair *) messageQueue->remove(0)) != NULL) {
    processMessage(messageLetPair->index, messageLetPair->message);
    messageLetPair->message->unreserve(this); // unreserve the message so that it can be reused by the issuing object
  }
  
  // process remainder of block
  processDspToIndex(blockSizeFloat);
  blockIndexOfLastMessage = 0.0f; // reset the block index of the last received message
}

void DspObject::resolveInputBuffersAtInlet(int inletIndex) {
  List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[inletIndex];
  int numConnections = incomingDspConnectionsList->size();
  
  switch (numConnections) {
    case 0: {
      break; // nothing to do
    }
    case 1: {
      ObjectLetPair *objectLetPair = (ObjectLetPair *) incomingDspConnectionsList->get(0);
      DspObject *remoteObject = (DspObject *) objectLetPair->object;
      localDspBufferAtInlet[inletIndex] = remoteObject->getDspBufferAtOutlet(objectLetPair->index);
      break;
    }
    default: { // numConnections > 1
      localDspBufferAtInlet[inletIndex] = localDspBufferAtInletReserved[inletIndex];
      float *localInputBuffer = localDspBufferAtInletReserved[inletIndex];
      
      // copy the single connection's output buffer to the input buffer
      ObjectLetPair *objectLetPair = (ObjectLetPair *) incomingDspConnectionsList->get(0);
      float *remoteOutputBuffer = ((DspObject *) objectLetPair->object)->getDspBufferAtOutlet(objectLetPair->index);
      memcpy(localInputBuffer, remoteOutputBuffer, numBytesInBlock);
      
      // add the remaining output buffers to the input buffer
      for (int j = 1; j < numConnections; j++) {
        objectLetPair = (ObjectLetPair *) incomingDspConnectionsList->get(j);
        remoteOutputBuffer = ((DspObject *) objectLetPair->object)->getDspBufferAtOutlet(objectLetPair->index);
        ArrayArithmetic::add(localInputBuffer, remoteOutputBuffer, localInputBuffer, 0, blockSizeInt);
      }
      break;
    }
  }
}

void DspObject::processDspToIndex(float blockIndex) {
  // by default, this function does nothing
}

bool DspObject::isRootNode() {
  if (!MessageObject::isRootNode()) {
    return false;
  } else {
    for (int i = 0; i < numDspInlets; i++) {
      if (incomingDspConnectionsListAtInlet[i]->size() > 0) {
        return false;
      }
    }
    return true;
  }
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
        ObjectLetPair *objectLetPair = (ObjectLetPair *) incomingMessageConnectionsListAtInlet[i]->get(j);
        List *parentProcessList = objectLetPair->object->getProcessOrder();
        processList->add(parentProcessList);
        delete parentProcessList;
      }
    }
    for (int i = 0; i < numDspInlets; i++) {
      for (int j = 0; j < incomingDspConnectionsListAtInlet[i]->size(); j++) {
        ObjectLetPair *objectLetPair = (ObjectLetPair *) incomingDspConnectionsListAtInlet[i]->get(j);
        List *parentProcessList = objectLetPair->object->getProcessOrder();
        processList->add(parentProcessList);
        delete parentProcessList;
      }
    }
    processList->add(this);
    return processList;
  }
}
