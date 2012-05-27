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

#include "MessageObject.h"
#include "PdGraph.h"

MessageObject::MessageObject(int numMessageInlets, int numMessageOutlets, PdGraph *graph) {
  this->graph = graph;
  this->isOrdered = false;
  canvasX = canvasY = 0.0f;

  // initialise incoming connections list
  incomingMessageConnections = vector<list<ObjectLetPair> >(numMessageInlets);
  
  // initialise outgoing connections list
  outgoingMessageConnections = vector<list<ObjectLetPair> >(numMessageOutlets);
}

MessageObject::~MessageObject() {
  // nothing to do
}

ConnectionType MessageObject::getConnectionType(int outletIndex) {
  return MESSAGE;
}

bool MessageObject::shouldDistributeMessageToInlets() {
  return true;
}

#pragma mark - Process Messages

void MessageObject::receiveMessage(int inletIndex, PdMessage *message) {
  int numMessageInlets = incomingMessageConnections.size();
  if (inletIndex == 0 &&
      numMessageInlets > 1 &&
      message->getNumElements() > 1 &&
      shouldDistributeMessageToInlets()) {
    // if the message should be distributed across the inlets
    int maxInletToDistribute = (message->getNumElements() < numMessageInlets)
        ? message->getNumElements() : numMessageInlets;
    PdMessage *distributedMessage = PD_MESSAGE_ON_STACK(1);
    for (int i = maxInletToDistribute-1; i >= 0; i--) { // send to right-most inlet first
      switch (message->getType(i)) {
        case FLOAT: {
          distributedMessage->initWithTimestampAndFloat(message->getTimestamp(), message->getFloat(i));
          break;
        }
        case SYMBOL: {
          distributedMessage->initWithTimestampAndSymbol(message->getTimestamp(), message->getSymbol(i));
          break;
        }
        case BANG: {
          distributedMessage->initWithTimestampAndBang(message->getTimestamp());
          break;
        }
        default: {
          break;
        }
      }
      processMessage(i, distributedMessage);
    }
  } else {
    // otherwise just send the message through normally
    processMessage(inletIndex, message);
  }
}

void MessageObject::sendMessage(int outletIndex, PdMessage *message) {
  list<ObjectLetPair>::iterator it = outgoingMessageConnections[outletIndex].begin();
  list<ObjectLetPair>::iterator end = outgoingMessageConnections[outletIndex].end();
  while (it != end) {
    ObjectLetPair objectLetPair = *it++;
    objectLetPair.first->receiveMessage(objectLetPair.second, message);
  }
}

void MessageObject::processMessage(int inletIndex, PdMessage *message) {
  // By default there is nothing to process.
}


#pragma mark - Manage Connections

void MessageObject::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  if (messageObject->getConnectionType(outletIndex) == MESSAGE) {
    list<ObjectLetPair> *connections = &incomingMessageConnections[inletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, outletIndex);
    connections->push_back(objectLetPair);
  }
}

void MessageObject::addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  // TODO(mhroth): it is assumed here that the input connection type of the destination object is MESSAGE. Correct?
  if (getConnectionType(outletIndex) == MESSAGE) {
    list<ObjectLetPair> *connections = &outgoingMessageConnections[outletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, inletIndex);
    connections->push_back(objectLetPair);
  }
}

void MessageObject::removeConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  // error check, the connection to this object must obviously be of type MESSAGE
  if (messageObject->getConnectionType(outletIndex) == MESSAGE) {
    list<ObjectLetPair> *incomingConnections = &incomingMessageConnections[inletIndex];
    ObjectLetPair objectLetPair = make_pair(messageObject, outletIndex);
    incomingConnections->remove(objectLetPair); // does this work?
  }
}

void MessageObject::removeConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  list<ObjectLetPair> *outgoingConnections = &outgoingMessageConnections[outletIndex];
  ObjectLetPair objectLetPair = make_pair(messageObject, inletIndex);
  outgoingConnections->remove(objectLetPair);
}

list<ObjectLetPair> MessageObject::getIncomingConnections(unsigned int inletIndex) {
  return incomingMessageConnections.empty() ? list<ObjectLetPair>() : incomingMessageConnections[inletIndex];
}

list<ObjectLetPair> MessageObject::getOutgoingConnections(unsigned int outletIndex) {
  return outgoingMessageConnections.empty() ? list<ObjectLetPair>() : outgoingMessageConnections[outletIndex];
}


#pragma mark - Accessors

unsigned int MessageObject::getNumInlets() {
  return incomingMessageConnections.size();
}

unsigned int MessageObject::getNumOutlets() {
  return outgoingMessageConnections.size();
}

ObjectType MessageObject::getObjectType() {
  return OBJECT_UNKNOWN;
}

PdGraph *MessageObject::getGraph() {
  return graph;
}

void MessageObject::getCanvasPosition(float *x, float *y) {
  *x = canvasX;
  *y = canvasY;
}

void MessageObject::setCanvasPosition(float x, float y) {
  canvasX = x;
  canvasY = y;
}

bool MessageObject::isLeafNode() {
  for (unsigned int i = 0; i < outgoingMessageConnections.size(); i++) {
    if (!outgoingMessageConnections[i].empty()) return false;
  }
  return true;
}

list<DspObject *> MessageObject::getProcessOrder() {
  if (isOrdered) {
    // if this object has already been ordered, then move on
    return list<DspObject *>();
  } else {
    isOrdered = true;
    list<DspObject *> processList;
    for (unsigned int i = 0; i < incomingMessageConnections.size(); i++) {
      for (list<ObjectLetPair>::iterator it = incomingMessageConnections[i].begin();
         it != incomingMessageConnections[i].end(); ++it) {
        ObjectLetPair objectLetPair = *it;
        list<DspObject *> parentProcessList = objectLetPair.first->getProcessOrder();
        processList.splice(processList.end(), parentProcessList); // append parentProcessList to processList
      }
    }
    // this object is not added to the process list as MessageObjects do not process audio
    return processList;
  }
}

void MessageObject::resetOrderedFlag() {
  isOrdered = false;
}

void MessageObject::updateIncomingMessageConnection(MessageObject *messageObject, int oldOutletIndex,
    int inletIndex, int newOutletIndex) {
  /*
  vector<ObjectLetPair> incomingMessageConnectionsList = incomingMessageConnections[inletIndex];
  for (int i = 0; i < incomingMessageConnectionsList.size(); i++) {
    ObjectLetPair *objectLetPair = (ObjectLetPair *) incomingMessageConnectionsList->get(i);
    if (objectLetPair->object == messageObject &&
        objectLetPair->index == oldOutletIndex) {
      objectLetPair->index = newOutletIndex;
      return;
    }
  }
  */
}

void MessageObject::updateOutgoingMessageConnection(MessageObject *messageObject, int oldInletIndex,
      int outletIndex, int newInletIndex) {
  /*
  List *outgoingMessageConnectionsList = (List *) outgoingMessageConnections[outletIndex];
  int numConnections = outgoingMessageConnectionsList->size();
  for (int i = 0; i < numConnections; i++) {
    ObjectLetPair *objectLetPair = (ObjectLetPair *) outgoingMessageConnectionsList->get(i);
    if (objectLetPair->object == messageObject &&
        objectLetPair->index == oldInletIndex) {
      objectLetPair->index = newInletIndex;
      return;
    }
  }
  */
}
