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

#include "MessageOutputObject.h"

MessageOutputObject::MessageOutputObject(int numOutlets, char *initString) : PdObject(initString) {
  this->numOutlets = numOutlets;
  messageOutletBuffers = (List **) malloc(numOutlets * sizeof(List *));
  for (int i = 0; i < numOutlets; i++) {
    messageOutletBuffers[i] = new List();
  }
  messagesAtOutlet = (int *) calloc(numOutlets, sizeof(int));
  numBytesInMessagesAtOutlet = numOutlets * sizeof(int);
}

MessageOutputObject::~MessageOutputObject() {
  for (int i = 0; i < numOutlets; i++) {
    List *list = messageOutletBuffers[i];
    for (int j = 0; j < list->getNumElements(); j++) {
      delete (PdMessage *) list->get(j);
    }
    delete messageOutletBuffers[i];
  }
  free(messageOutletBuffers);
  free(messagesAtOutlet);
}

PdObjectType MessageOutputObject::getObjectType() {
  return MESSAGE;
}

PdMessage *MessageOutputObject::getNextOutgoingMessage(int outletIndex) {
  List *list = messageOutletBuffers[outletIndex];
  PdMessage *message = (PdMessage *) list->get(messagesAtOutlet[outletIndex]);
  if (message == NULL) {
    message = newCanonicalMessage();
    list->add(message);
  }
  messagesAtOutlet[outletIndex]++;
  return message;
}

void MessageOutputObject::setNextOutgoingMessage(int outletIndex, PdMessage *message) {
  List *list = messageOutletBuffers[outletIndex];
  if (messagesAtOutlet[outletIndex] < list->getNumElements()) {
    list->replace(messagesAtOutlet[outletIndex], message);
  } else {
    list->add(message);
  }
  messagesAtOutlet[outletIndex]++;
}


void MessageOutputObject::resetOutgoingMessageBuffers() {
  memset(messagesAtOutlet, 0, numBytesInMessagesAtOutlet);
}

PdMessage *MessageOutputObject::getMessageAtOutlet(int outletIndex, int messageIndex) {
  if (outletIndex < 0 || outletIndex >= numOutlets ||
      messageIndex < 0 || messageIndex >= messagesAtOutlet[outletIndex]) {
    return NULL;
  } else {
    return (PdMessage *) messageOutletBuffers[outletIndex]->get(messageIndex);
  }
}
