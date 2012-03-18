/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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
  codepath = DSP_OBJECT_PROCESS_NO_MESSAGE;
  
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
  
  // initialise the outgoing dsp connections list
  outgoingDspConnections = vector<list<ObjectLetPair> >(numDspOutlets);

  dspBufferAtInlet = (numDspInlets > 0) ? (float **) calloc(numDspInlets, sizeof(float *)) : NULL;
  
  // initialise the local output audio buffers
  dspBufferAtOutlet = (numDspOutlets > 0) ? (float **) calloc(numDspOutlets, sizeof(float)) : NULL;
  for (int i = 0; i < numDspOutlets; ++i) {
    dspBufferAtOutlet[i] = (float *) valloc(blockSizeInt * sizeof(float));
    memset(dspBufferAtOutlet[i], 0, blockSize * sizeof(float));
  }
}

DspObject::~DspObject() {  
  if (--zeroBufferCount == 0) {
    free(zeroBuffer);
    zeroBuffer = NULL;
    zeroBufferSize = 0;
  }
  
  // delete any messages still pending
  clearMessageQueue();
  
  // delete outlet buffers
  for (int i = 0; i < outgoingDspConnections.size(); i++) {
    if (i > 0 || (i == 0 && (incomingDspConnections.size() == 0 ||
        (incomingDspConnections.size() > 0 && dspBufferAtInlet[0] != dspBufferAtOutlet[0])))) {
      free(dspBufferAtOutlet[i]);
    }
  }
  
  // free the inlet buffer index
  free(dspBufferAtInlet);
  
  // free the outlet buffer index
  free(dspBufferAtOutlet);
}

const char *DspObject::getObjectLabel() {
  return "obj~";
}


#pragma mark -

ConnectionType DspObject::getConnectionType(int outletIndex) {
  return DSP;
}

float *DspObject::getDspBufferAtOutlet(int outletIndex) {
  // sanity check on outletIndex
  return (outletIndex < outgoingDspConnections.size())
      ? dspBufferAtOutlet[outletIndex] : NULL;
}

bool DspObject::mayReuseBuffer(DspObject *dspObject, unsigned int outletIndex) {
  return outgoingDspConnections[outletIndex].front().first == dspObject;
}

list<ObjectLetPair> DspObject::getIncomingConnections(unsigned int inletIndex) {
  list<ObjectLetPair> messageConnectionList = MessageObject::getIncomingConnections(inletIndex);
  list<ObjectLetPair> dspConnectionList = incomingDspConnections.empty()
      ? list<ObjectLetPair>() : incomingDspConnections[inletIndex];
  messageConnectionList.insert(messageConnectionList.end(), dspConnectionList.begin(), dspConnectionList.end());
  return messageConnectionList;
}

list<ObjectLetPair> DspObject::getIncomingDspConnections(unsigned int inletIndex) {
  return (inletIndex < incomingDspConnections.size()) ? incomingDspConnections[inletIndex] : list<ObjectLetPair>();
}

list<ObjectLetPair> DspObject::getOutgoingConnections(unsigned int outletIndex) {
  list<ObjectLetPair> messageConnectionList = MessageObject::getOutgoingConnections(outletIndex);
  list<ObjectLetPair> dspConnectionList = outgoingDspConnections.empty()
      ? list<ObjectLetPair>() : outgoingDspConnections[outletIndex];
  messageConnectionList.insert(messageConnectionList.end(), dspConnectionList.begin(), dspConnectionList.end());
  return messageConnectionList;
}

list<ObjectLetPair> DspObject::getOutgoingDspConnections(unsigned int outletIndex) {
  return outgoingDspConnections[outletIndex];
}


#pragma mark - Add/Remove Connections

void DspObject::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  MessageObject::addConnectionFromObjectToInlet(messageObject, outletIndex, inletIndex);
  
  if (messageObject->getConnectionType(outletIndex) == DSP) {
    list<ObjectLetPair> *connections = &incomingDspConnections[inletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, outletIndex);
    connections->push_back(objectLetPair);
  }
  
  onInletConnectionUpdate(inletIndex);
}

void DspObject::removeConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  if (messageObject->getConnectionType(outletIndex) == DSP) {
    list<ObjectLetPair> *connections = &incomingDspConnections[inletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, outletIndex);
    connections->remove(objectLetPair); // NOTE(mhroth): does this work?
  } else {
    MessageObject::removeConnectionFromObjectToInlet(messageObject, outletIndex, inletIndex);
  }
  
  onInletConnectionUpdate(inletIndex);
}

void DspObject::onInletConnectionUpdate(unsigned int inletIndex) {
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

void DspObject::setDspBufferAtInletWithReuse(float *buffer, unsigned int inletIndex, bool mayReuse) {
  // check to see if all outgoing connections are to the same object
  // if not, then the input buffer cannot be reused as an output buffer
  bool allConnectionsAreToSameObject = outgoingDspConnections.size() == 1 && outgoingDspConnections[0].size() > 0;
  if (allConnectionsAreToSameObject) {
    list<ObjectLetPair> connections = outgoingDspConnections[0];
    MessageObject *object = connections.front().first;
    for (list<ObjectLetPair>::iterator it = ++(connections.begin()); it != connections.end(); ++it) {
      if ((*it).first != object) {
        allConnectionsAreToSameObject = false;
        break;
      }
    }
  }
  
  // if it is possible to reuse the input buffer, do so
  if (inletIndex == 0 &&
      mayReuse && canReuseInputBuffer() &&
      allConnectionsAreToSameObject &&
      buffer != DspObject::zeroBuffer) { // don't overwrite the zero buffer
    // if the input and output buffers are not already the same, then free the output buffer
    if (dspBufferAtInlet[0] != dspBufferAtOutlet[0]) free(dspBufferAtOutlet[0]);
    
    dspBufferAtInlet[0] = buffer; // set the incoming buffer
    dspBufferAtOutlet[0] = buffer;
  } else {
    if (inletIndex == 0 && 
        outgoingDspConnections.size() > 0 &&
        dspBufferAtInlet[0] == dspBufferAtOutlet[0]) {
      dspBufferAtOutlet[0] = (float *) valloc(blockSizeInt * sizeof(float));
    }
    dspBufferAtInlet[inletIndex] = buffer; // set the incoming buffer
  }
  
  // if the object must take any special steps when updating the input buffer, do so now
  onDspBufferAtInletUpdate(buffer, inletIndex);
}


#pragma mark -

void DspObject::clearMessageQueue() {
  while (!messageQueue.empty()) {
    MessageLetPair messageLetPair = messageQueue.front();
    PdMessage *message = messageLetPair.first;
    message->freeMessage();
    messageQueue.pop();
  }
}

void DspObject::receiveMessage(int inletIndex, PdMessage *message) {
  // Queue the message to be processed during the DSP round only if the graph is switched on.
  // Otherwise messages would begin to pile up because the graph is not processed.
  if (graph->isSwitchedOn()) {
    // Copy the message to the heap so that it is available to process later.
    // The message is released once it is consumed in processDsp().
    messageQueue.push(make_pair(message->copyToHeap(), inletIndex));
    codepath = DSP_OBJECT_PROCESS_MESSAGE;
  }
}

void DspObject::processDsp() {
  switch (codepath) {
    case DSP_OBJECT_PROCESS_MESSAGE: {
      double blockIndexOfLastMessage = 0.0; // reset the block index of the last received message
      do { // there is at least one message
        MessageLetPair messageLetPair = messageQueue.front();
        PdMessage *message = messageLetPair.first;
        unsigned int inletIndex = messageLetPair.second;
        
        double blockIndexOfCurrentMessage = graph->getBlockIndex(message);
        processDspWithIndex(blockIndexOfLastMessage, blockIndexOfCurrentMessage);
        processMessage(inletIndex, message);
        message->freeMessage(); // free the message from the head, the message has been consumed.
        messageQueue.pop();
        
        blockIndexOfLastMessage = blockIndexOfCurrentMessage;
      } while (!messageQueue.empty());
      processDspWithIndex(blockIndexOfLastMessage, (double) blockSizeInt);
      codepath = DSP_OBJECT_PROCESS_NO_MESSAGE;
      break;
    }
    default:
    case DSP_OBJECT_PROCESS_NO_MESSAGE: {
      processDspWithIndex(0, blockSizeInt);
      break;
    }
  }
}

void DspObject::processDspWithIndex(double fromIndex, double toIndex) {
  // by default, this function just calls the integer version with adjusted block indicies
  processDspWithIndex((int) ceil(fromIndex), (int) ceil(toIndex));
}

void DspObject::processDspWithIndex(int fromIndex, int toIndex) {
  // by default, this function just calls the float version
  processDspWithIndex((float) fromIndex, (float) toIndex);
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
