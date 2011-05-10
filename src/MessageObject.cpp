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

#include "MessageObject.h"
#include "PdGraph.h"

MessageObject::MessageObject(int numMessageInlets, int numMessageOutlets, PdGraph *graph) {
  this->numMessageInlets = numMessageInlets;
  this->numMessageOutlets = numMessageOutlets;
  this->graph = graph;
  this->isOrdered = false;

  // initialise incoming connections list
  // while malloc(0) does work well with free(), it also seems to use some small amount of memory.
  // thus numMessageInlets is manually checked for zero and a NULL pointer is returned.
  incomingMessageConnectionsListAtInlet =
      (numMessageInlets > 0) ? (List **) malloc(numMessageInlets * sizeof(List *)) : NULL;
  for (int i = 0; i < numMessageInlets; i++) {
    incomingMessageConnectionsListAtInlet[i] = new List();
  }

  // initialise outgoing connections list
  outgoingMessageConnectionsListAtOutlet =
      (numMessageOutlets > 0) ? (List **) malloc(numMessageOutlets * sizeof(List *)) : NULL;
  for (int i = 0; i < numMessageOutlets; i++) {
    outgoingMessageConnectionsListAtOutlet[i] = new List();
  }
}

MessageObject::~MessageObject() {
  // delete incoming connections list
  for (int i = 0; i < numMessageInlets; i++) {
    List *list = incomingMessageConnectionsListAtInlet[i];
    for (int j = 0; j < list->size(); j++) {
      free(list->get(j));
    }
    delete list;
  }
  free(incomingMessageConnectionsListAtInlet);

  // delete outgoing connections list
  for (int i = 0; i < numMessageOutlets; i++) {
    List *list = outgoingMessageConnectionsListAtOutlet[i];
    for (int j = 0; j < list->size(); j++) {
      free(list->get(j));
    }
    delete list;
  }
  free(outgoingMessageConnectionsListAtOutlet);
}

ConnectionType MessageObject::getConnectionType(int outletIndex) {
  return MESSAGE;
}

bool MessageObject::shouldDistributeMessageToInlets() {
  return true;
}

void MessageObject::receiveMessage(int inletIndex, PdMessage *message) {
  if (shouldDistributeMessageToInlets() &&
      inletIndex == 0 &&
      numMessageInlets > 1 &&
      message->getNumElements() > 1) {
    // if the message should be distributed across the inlets
    int maxInletToDistribute = (message->getNumElements() < numMessageInlets)
        ? message->getNumElements() : numMessageInlets;
    for (int i = maxInletToDistribute-1; i >= 0; i--) { // send to right-most inlet first
      PdMessage *distributedMessage = NULL;
      switch (message->getType(i)) {
        case FLOAT: {
          distributedMessage = PD_MESSAGE_ON_STACK(1);
          distributedMessage->initWithTimestampAndFloat(message->getTimestamp(), message->getFloat(i));
          break;
        }
        case SYMBOL: {
          distributedMessage = PD_MESSAGE_ON_STACK(1);
          // TODO(mhroth): properly init symbol on stack
          break;
        }
        case BANG: {
          distributedMessage = PD_MESSAGE_ON_STACK(1);
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
  List *outgoingMessageConnectionsList = outgoingMessageConnectionsListAtOutlet[outletIndex];
  int numConnectionsAtOutlet = outgoingMessageConnectionsList->size();
  for (int i = 0; i < numConnectionsAtOutlet; i++) {
    ObjectLetPair *objectLetPair = (ObjectLetPair *) outgoingMessageConnectionsList->get(i);
    objectLetPair->object->receiveMessage(objectLetPair->index, message);
  }
}

void MessageObject::processMessage(int inletIndex, PdMessage *message) {
  // By default there is nothing to process.
}

bool MessageObject::doesProcessAudio() {
  return false;
}

void MessageObject::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  if (messageObject->getConnectionType(outletIndex) == MESSAGE) {
    List *incomingMessageConnectionsList = incomingMessageConnectionsListAtInlet[inletIndex];
    ObjectLetPair *objectLetPair = (ObjectLetPair *) malloc(sizeof(ObjectLetPair));
    objectLetPair->object = messageObject;
    objectLetPair->index = outletIndex;
    incomingMessageConnectionsList->add(objectLetPair);
  }
}

void MessageObject::addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  // TODO(mhroth): it is assumed here that the input connection type of the destination object is MESSAGE. Correct?
  if (getConnectionType(outletIndex) == MESSAGE) {
    List *outgoingMessageConnectionsList = outgoingMessageConnectionsListAtOutlet[outletIndex];
    ObjectLetPair *objectLetPair = (ObjectLetPair *) malloc(sizeof(ObjectLetPair));
    objectLetPair->object = messageObject;
    objectLetPair->index = inletIndex;
    outgoingMessageConnectionsList->add(objectLetPair);
  }
}

ObjectType MessageObject::getObjectType() {
  return OBJECT_UNKNOWN;
}

bool MessageObject::isLeafNode() {
  for (int i = 0; i < numMessageOutlets; i++) {
    if (outgoingMessageConnectionsListAtOutlet[i]->size() > 0) {
      return false;
    }
  }
  return true;
}

List *MessageObject::getProcessOrder() {
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
    processList->add(this);
    return processList;
  }
}

void MessageObject::resetOrderedFlag() {
  isOrdered = false;
}

void MessageObject::updateIncomingMessageConnection(MessageObject *messageObject, int oldOutletIndex,
    int inletIndex, int newOutletIndex) {
  List *incomingMessageConnectionsList = (List *) incomingMessageConnectionsListAtInlet[inletIndex];
  int numConnections = incomingMessageConnectionsList->size();
  for (int i = 0; i < numConnections; i++) {
    ObjectLetPair *objectLetPair = (ObjectLetPair *) incomingMessageConnectionsList->get(i);
    if (objectLetPair->object == messageObject &&
        objectLetPair->index == oldOutletIndex) {
      objectLetPair->index = newOutletIndex;
      return;
    }
  }
}

void MessageObject::updateOutgoingMessageConnection(MessageObject *messageObject, int oldInletIndex,
      int outletIndex, int newInletIndex) {
  List *outgoingMessageConnectionsList = (List *) outgoingMessageConnectionsListAtOutlet[outletIndex];
  int numConnections = outgoingMessageConnectionsList->size();
  for (int i = 0; i < numConnections; i++) {
    ObjectLetPair *objectLetPair = (ObjectLetPair *) outgoingMessageConnectionsList->get(i);
    if (objectLetPair->object == messageObject &&
        objectLetPair->index == oldInletIndex) {
      objectLetPair->index = newInletIndex;
      return;
    }
  }
}
