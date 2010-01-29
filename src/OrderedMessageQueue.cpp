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

#include "OrderedMessageQueue.h"

OrderedMessageQueue::OrderedMessageQueue() : LinkedList() {
  // nothing to do
}

OrderedMessageQueue::~OrderedMessageQueue() {
  // nothing to do
}

void OrderedMessageQueue::insertMessage(MessageObject *messageObject, int outletIndex, PdMessage *message) {
  LinkedListNode *newNode = getEmptyNode();
  MessageDestination *destination = (MessageDestination *) newNode->data;
  destination->object = messageObject;
  destination->message = message;
  destination->index = outletIndex;
  
  LinkedListNode *node = head;
  while (node != NULL) {
    destination = (MessageDestination *) node->data;
    if (message->getTimestamp() < destination->message->getTimestamp()) {
      insertBefore(newNode, node);
      return;
    } else {
      node = node->next;
    }
  }
  insertAfter(newNode, tail);
}

void *OrderedMessageQueue::newDataHolder() {
  return malloc(sizeof(MessageDestination));
}

void OrderedMessageQueue::deleteDataHolder(void *data) {
  free((MessageDestination *) data);
}
