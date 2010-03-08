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

#include "MessageObject.h"
#include "PdGraph.h"

MessageObject::MessageObject(int numMessageInlets, int numMessageOutlets, PdGraph *graph) {
  this->numMessageInlets = numMessageInlets;
  this->numMessageOutlets = numMessageOutlets;
  this->graph = graph;
  this->isOrdered = false;
  
  // initialise incoming connections list
  incomingMessageConnectionsListAtInlet = (List **) malloc(numMessageInlets * sizeof(List *));
  for (int i = 0; i < numMessageInlets; i++) {
    incomingMessageConnectionsListAtInlet[i] = new List();
  }
  
  // initialise outgoing connections list
  outgoingMessageConnectionsListAtOutlet = (List **) malloc(numMessageOutlets * sizeof(List *));
  for (int i = 0; i < numMessageOutlets; i++) {
    outgoingMessageConnectionsListAtOutlet[i] = new List();
  }
  
  // initialise outgoing message pool
  messageOutletPools = (List **) malloc(numMessageOutlets * sizeof(List *));
  for (int i = 0; i < numMessageOutlets; i++) {
    messageOutletPools[i] = new List();
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
  
  // delete outgoing message pools
  for (int i = 0; i < numMessageOutlets; i++) {
    List *messageOutletPool = (List *) messageOutletPools[i];
    // delete the contents of the pool
    for (int j = 0; j < messageOutletPool->size(); j++) {
      PdMessage *message = (PdMessage *) messageOutletPool->get(j);
      delete message;
    }
    delete messageOutletPools[i];
  }
  free(messageOutletPools);
}

ConnectionType MessageObject::getConnectionType(int outletIndex) { 
  return MESSAGE;
}

void MessageObject::receiveMessage(int inletIndex, PdMessage *message) {
  processMessage(inletIndex, message);
}

void MessageObject::sendMessage(int outletIndex, PdMessage *message) {
  List *outgoingMessageConnectionsList = outgoingMessageConnectionsListAtOutlet[outletIndex];
  int numConnectionsAtOutlet = outgoingMessageConnectionsList->size();
  for (int i = 0; i < numConnectionsAtOutlet; i++) {
    ObjectLetPair *objectLetPair = (ObjectLetPair *) outgoingMessageConnectionsList->get(i);
    objectLetPair->object->receiveMessage(objectLetPair->index, message);
  }
}

void MessageObject::sendScheduledMessage(int outletIndex, PdMessage *message) {
  sendMessage(outletIndex, message);
  
  postSendMessageHook(outletIndex, message);
}

void MessageObject::postSendMessageHook(int outletIndex, PdMessage *message) {
  // does nothing by default
}

void MessageObject::processMessage(int inletIndex, PdMessage *message) {
  // By default there is nothing to process.
}

void MessageObject::processDsp() {
  // By default no audio is processed.
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

PdMessage *MessageObject::getNextOutgoingMessage(int outletIndex) {
  List *messageOutletPool = messageOutletPools[outletIndex];
  int numMessagesInPool = messageOutletPool->size();
  PdMessage *message = NULL;
  for (int i = 0; i < numMessagesInPool; i++) {
    message = (PdMessage *) messageOutletPool->get(i);
    if (!message->isReserved()) {
      return message;
    }
  }
  message = newCanonicalMessage(outletIndex);
  messageOutletPool->add(message);
  return message;
}

PdMessage *MessageObject::newCanonicalMessage(int outletIndex) {
  // default implementation returns a message with one element
  PdMessage *outgoingMessage = new PdMessage();
  outgoingMessage->addElement(new MessageElement());
  return outgoingMessage;
}

bool MessageObject::isRootNode() {
  if (strcmp(getObjectLabel(), "receive") == 0) {
    return true;
  }
  for (int i = 0; i < numMessageInlets; i++) {
    if (incomingMessageConnectionsListAtInlet[i]->size() > 0) {
      return false;
    }
  }
  return true;
}

bool MessageObject::isLeafNode() {
  if (strcmp(getObjectLabel(), "send") == 0) {
    return true;
  }
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
    if (!isRootNode()) {
      for (int i = 0; i < numMessageInlets; i++) {
        for (int j = 0; j < incomingMessageConnectionsListAtInlet[i]->size(); j++) {
          ObjectLetPair *objectLetPair = (ObjectLetPair *) incomingMessageConnectionsListAtInlet[i]->get(j);
          List *parentProcessList = objectLetPair->object->getProcessOrder();
          processList->add(parentProcessList);
          delete parentProcessList;
        }
      }
    }
    processList->add(this);
    return processList;
  }
}