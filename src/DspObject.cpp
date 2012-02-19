/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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


#pragma mark - Constructor/Destructor

DspObject::DspObject(int numMessageInlets, int numDspInlets, int numMessageOutlets, int numDspOutlets, PdGraph *graph) :
    MessageObject(numMessageInlets, numMessageOutlets, graph) {
  init(numDspInlets, numDspOutlets, graph->getBlockSize());
}

DspObject::DspObject(int numMessageInlets, int numDspInlets, int numMessageOutlets, int numDspOutlets, int blockSize, PdGraph *graph) : 
    MessageObject(numMessageInlets, numMessageOutlets, graph) {
  init(numDspInlets, numDspOutlets, blockSize);
}

void DspObject::init(int numDspInlets, int numDspOutlets, int blockSize) {
  blockSizeInt = blockSize;
  blockIndexOfLastMessage = 0.0f;
  signalPrecedence = MESSAGE_MESSAGE; // default
  numBytesInBlock = blockSizeInt * sizeof(float);
  
  if (zeroBufferSize < blockSize) {
    float *buffer = (float *) realloc(zeroBuffer, blockSize * sizeof(float));
    if (buffer == NULL) buffer = (float *) malloc(blockSize * sizeof(float));
    zeroBuffer = buffer;
    memset(zeroBuffer, 0, blockSize * sizeof(float));
    zeroBufferSize = blockSize;
  }
  zeroBufferCount++;
  
  // initialise the incoming dsp connections list
  incomingDspConnections = vector<list<ObjectLetPair> >(numDspInlets);
  
  // with no connections, this convenience pointer should point to zero
  dspBufferAtInlet0 = zeroBuffer;
  
  // initialise the outgoing dsp connections list
  outgoingDspConnections = vector<list<ObjectLetPair> >(numDspOutlets);
  
  // with no connections, this convenience pointer should point to zero
  dspBufferAtInlet1 = zeroBuffer;
  
  dspBufferAtInlet = (numDspInlets > 2) ? (float **) calloc(numDspInlets, sizeof(float *)) : NULL;
  dspBufferRefListAtInlet = vector<vector<float *> >(numDspInlets);
  
  // initialise the local output audio buffers
  dspBufferAtOutlet0 = (numDspOutlets > 0) ? (float *) calloc(numDspOutlets * blockSize, sizeof(float)) : NULL;
  dspBufferAtOutlet0 = (numDspOutlets > 0) ? (float *) valloc(numDspOutlets * blockSize * sizeof(float)) : NULL;
  memset(dspBufferAtOutlet0, 0, numDspOutlets * blockSize * sizeof(float)); // clear the block
}

DspObject::~DspObject() {  
  if (--zeroBufferCount == 0) {
    free(zeroBuffer);
    zeroBuffer = NULL;
    zeroBufferSize = 0;
  }

  free(dspBufferAtInlet);
  
  // free the local output audio buffers
  free(dspBufferAtOutlet0);
}


#pragma mark -

ConnectionType DspObject::getConnectionType(int outletIndex) {
  return DSP;
}

float *DspObject::getDspBufferRefAtOutlet(int outletIndex) {
  // sanity check on outletIndex
  return (outletIndex < outgoingDspConnections.size()) ? dspBufferAtOutlet0 + (outletIndex * blockSizeInt) : NULL;
}

bool DspObject::doesProcessAudio() {
  return true;
}

list<ObjectLetPair> DspObject::getIncomingConnections(unsigned int inletIndex) {
  list<ObjectLetPair> messageConnectionList = MessageObject::getIncomingConnections(inletIndex);
  list<ObjectLetPair> dspConnectionList = incomingDspConnections.empty()
      ? list<ObjectLetPair>() : incomingDspConnections[inletIndex];
  messageConnectionList.insert(messageConnectionList.end(), dspConnectionList.begin(), dspConnectionList.end());
  return messageConnectionList;
}

list<ObjectLetPair> DspObject::getOutgoingConnections(unsigned int outletIndex) {
  list<ObjectLetPair> messageConnectionList = MessageObject::getOutgoingConnections(outletIndex);
  list<ObjectLetPair> dspConnectionList = outgoingDspConnections.empty()
      ? list<ObjectLetPair>() : outgoingDspConnections[outletIndex];
  messageConnectionList.insert(messageConnectionList.end(), dspConnectionList.begin(), dspConnectionList.end());
  return messageConnectionList;
}


#pragma mark - Add/Remove Connections

void DspObject::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  MessageObject::addConnectionFromObjectToInlet(messageObject, outletIndex, inletIndex);
  
  if (messageObject->getConnectionType(outletIndex) == DSP) {
    list<ObjectLetPair> *connections = &incomingDspConnections[inletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, outletIndex);
    connections->push_back(objectLetPair);
    
    // set signal precedence
    signalPrecedence = (DspMessagePresedence) (signalPrecedence | (0x1 << inletIndex));
    
    DspObject *dspObject = (DspObject *) messageObject;
    // get pointer to inletIndex-th element of dspBufferRefListAtInlet
    vector<float *> *dspBufferRefList = &(*(dspBufferRefListAtInlet.begin() + inletIndex));
    dspBufferRefList->push_back(dspObject->getDspBufferRefAtOutlet(outletIndex));
    updateInletBufferRefs(inletIndex);
  }
}

void DspObject::removeConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  if (messageObject->getConnectionType(outletIndex) == DSP) {
    list<ObjectLetPair> *incomingConnections = &incomingDspConnections[inletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, outletIndex);
    incomingConnections->remove(objectLetPair); // does this work?
    
    updateInletBufferRefs(inletIndex);
  } else {
    MessageObject::removeConnectionFromObjectToInlet(messageObject, outletIndex, inletIndex);
  }
}

void DspObject::updateInletBufferRefs(unsigned int inletIndex) {
  vector<float *> *dspBufferRefList = &(*(dspBufferRefListAtInlet.begin() + inletIndex));
  if (inletIndex == 0) {
    if (incomingDspConnections[0].size() == 0) {
      dspBufferAtInlet0 = zeroBuffer;
    } else if (incomingDspConnections[0].size() == 1) {
      dspBufferAtInlet0 = dspBufferRefList->at(0);
    }
  } else if (inletIndex == 1) {
    if (incomingDspConnections.size() == 0) {
      dspBufferAtInlet1 = zeroBuffer;
    } else if (incomingDspConnections[1].size() == 1) {
      dspBufferAtInlet1 = dspBufferRefList->at(0);
    }
  }
  
  onInletConnectionUpdate();
}

void DspObject::onInletConnectionUpdate() {
  // nothing to do
}

void DspObject::addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  MessageObject::addConnectionToObjectFromOutlet(messageObject, inletIndex, outletIndex);
  
  // TODO(mhroth): it is assumed here that the input connection type of the destination object is DSP. Correct?
  if (getConnectionType(outletIndex) == DSP) {
    list<ObjectLetPair> *connections = &outgoingDspConnections[outletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, inletIndex);
    connections->push_back(objectLetPair);
  }
}

void DspObject::removeConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  if (getConnectionType(outletIndex) == MESSAGE) {
    MessageObject::removeConnectionToObjectFromOutlet(messageObject, inletIndex, outletIndex);
  } else {
    list<ObjectLetPair> *outgoingConnections = &outgoingDspConnections[outletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, inletIndex);
    outgoingConnections->remove(objectLetPair);
  }
}


#pragma mark -

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
  // take care of common special cases giving a good increase in speed. Most objects have only one
  // or two inlets. And most objects usually have only one (or none!) DSP connection per inlet.
  int numDspInlets = incomingDspConnections.size();
  switch (numDspInlets) {
    default: {
      for (int i = 2; i < numDspInlets; i++) {
        vector<float *> *dspBufferRefList = &(*(dspBufferRefListAtInlet.begin() + i));
        if (dspBufferRefList->size() > 1) {
          dspBufferAtInlet[i] = (float *) alloca(numBytesInBlock);
          resolveInputBuffers(i, dspBufferAtInlet[i]);
        }
      }
      // allow fallthrough
    }
    case 2: {
      // numConnectionsToInlet1
      if (incomingDspConnections[1].size() > 1) {
        dspBufferAtInlet1 = (float *) alloca(numBytesInBlock);
        resolveInputBuffers(1, dspBufferAtInlet1);
      }
      // allow fallthrough
    }
    case 1: {
      RESOLVE_DSPINLET0_IF_NECESSARY();
      // allow fallthrough
    }
    case 0: break;
  }
  
  if (!messageQueue.empty()) {
    do { // there is at least one message
      MessageLetPair messageLetPair = messageQueue.front();
      PdMessage *message = messageLetPair.first;
      unsigned int inletIndex = messageLetPair.second;
      
      float blockIndexOfCurrentMessage = graph->getBlockIndex(message);
      processDspWithIndex(blockIndexOfLastMessage, blockIndexOfCurrentMessage);
      processMessage(inletIndex, message);
      message->freeMessage(); // free the message from the head, the message has been consumed.
      messageQueue.pop();
      
      blockIndexOfLastMessage = blockIndexOfCurrentMessage;
    } while (!messageQueue.empty());
    processDspWithIndex(blockIndexOfLastMessage, (float) blockSizeInt);
    blockIndexOfLastMessage = 0.0f; // reset the block index of the last received message
  } else {
    processDspWithIndex(0, blockSizeInt);
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

unsigned int DspObject::getNumInlets() {
  int numMessageConnections = incomingMessageConnections.size();
  int numDspConnections = incomingDspConnections.size();
  return (numMessageConnections > numDspConnections) ? numMessageConnections : numDspConnections;
}

unsigned int DspObject::getNumOutlets() {
  int numMessageConnections = outgoingMessageConnections.size();
  int numDspConnections = outgoingDspConnections.size();
  return (numMessageConnections > numDspConnections) ? numMessageConnections : numDspConnections;
}

bool DspObject::isLeafNode() {
  if (!MessageObject::isLeafNode()) {
    return false;
  } else {
    for (int i = 0; i < outgoingDspConnections.size(); i++) {
      if (outgoingDspConnections[i].size() > 0) {
        return false;
      }
    }
    return true;
  }
}

list<MessageObject *> *DspObject::getProcessOrder() {
  if (isOrdered) {
    // if this object has already been ordered, then move on
    return new list<MessageObject *>();
  } else {
    isOrdered = true;
    list<MessageObject *> *processList = new list<MessageObject *>();
    for (int i = 0; i < incomingMessageConnections.size(); i++) {
      list<ObjectLetPair>::iterator it = incomingMessageConnections[i].begin();
      list<ObjectLetPair>::iterator end = incomingMessageConnections[i].end();
      while (it != end) {
        ObjectLetPair objectLetPair = *it++;
        list<MessageObject *> *parentProcessList = objectLetPair.first->getProcessOrder();
        processList->splice(processList->end(), *parentProcessList);
        delete parentProcessList;
      }
    }
    for (int i = 0; i < incomingDspConnections.size(); i++) {
      list<ObjectLetPair>::iterator it = incomingDspConnections[i].begin();
      list<ObjectLetPair>::iterator end = incomingDspConnections[i].end();
      while (it != end) {
        ObjectLetPair objectLetPair = *it++;
        list<MessageObject *> *parentProcessList = objectLetPair.first->getProcessOrder();
        processList->splice(processList->end(), *parentProcessList);
        delete parentProcessList;
      }
    }
    processList->push_back(this);
    return processList;
  }
}
