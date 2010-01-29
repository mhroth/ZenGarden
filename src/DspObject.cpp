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

#include "DspObject.h"

DspObject::DspObject(int numMessageInlets, int numDspInlets, int numMessageOutlets, int numDspOutlets, PdGraph *graph) : MessageObject(numMessageInlets, numMessageOutlets, graph) {
  this->numDspInlets = numDspInlets;
  this->numDspOutlets = numDspOutlets;
  blockSizeInt = graph->getBlockSize();
  blockSizeFloat = (float) blockSizeInt;
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
  for (int i = 0; i < numDspInlets; i++) {
    localDspBufferAtInlet[i] = (float *) malloc(blockSizeInt * sizeof(float));
  }
  
  // initialise the local output audio buffers
  localDspBufferAtOutlet = (float **) malloc(numDspOutlets * sizeof(float *));
  for (int i = 0; i < numDspOutlets; i++) {
    localDspBufferAtOutlet[i] = (float *) malloc(blockSizeInt * sizeof(float));
  }
}

DspObject::~DspObject() {
  delete messageQueue;
  
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
  
  // free the local input audio buffers
  for (int i = 0; i < numDspInlets; i++) {
    free(localDspBufferAtInlet[i]);
  }
  free(localDspBufferAtInlet);
  
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

float *DspObject::getDspBufferAtOutlet(int outletIndex) {
  return localDspBufferAtOutlet[outletIndex];
}

void DspObject::receiveMessage(int inletIndex, PdMessage *message) {
  if (graph->isSwitchedOn()) {
    // Queue the message to be processed during the DSP round only if the graph is switched on.
    // Otherwise messages would begin to pile up.
    message->reserve(this); // reserve the message so that it won't be reused by the issuing object
    messageQueue->add(inletIndex, message);
  }
}

void DspObject::processDsp() {
  // collect all incoming audio into local input buffers
  static int numConnections = 0;
  static List *incomingDspConnectionsList = NULL;
  static ObjectLetPair *objectLetPair = NULL;
  static float *remoteOutputBuffer = NULL;
  static float *localInputBuffer = NULL;
  for (int i = 0; i < numDspInlets; i++) {
    incomingDspConnectionsList = incomingDspConnectionsListAtInlet[i];
    numConnections = incomingDspConnectionsList->size();
    localInputBuffer = localDspBufferAtInlet[i];
    
    switch (numConnections) {
      case 0: {
        break; // nothing to do
      }
      default: {
        objectLetPair = (ObjectLetPair *) incomingDspConnectionsList->get(0);
        remoteOutputBuffer = ((DspObject *) objectLetPair->object)->getDspBufferAtOutlet(objectLetPair->index);
        memcpy(localInputBuffer, remoteOutputBuffer, numBytesInBlock);
        for (int j = 1; j < numConnections; j++) {
          objectLetPair = (ObjectLetPair *) incomingDspConnectionsList->get(j);
          remoteOutputBuffer = ((DspObject *) objectLetPair->object)->getDspBufferAtOutlet(objectLetPair->index);
          for (int k = 0; k < blockSizeInt; k++) {
            localInputBuffer[k] += remoteOutputBuffer[k];
          }
        }
        break;
      }
    }
  }
  
  // process all pending messages in this block
  static MessageLetPair *messageLetPair = NULL;
  while ((messageLetPair = (MessageLetPair *) messageQueue->remove(0)) != NULL) {
    processMessage(messageLetPair->index, messageLetPair->message);
    messageLetPair->message->unreserve(this); // unreserve the message so that it can be reused by the issuing object
  }
  
  // process remainder of block
  processDspToIndex(blockSizeFloat);
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
