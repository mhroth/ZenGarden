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

#include "OrderedMessageQueue.h"

OrderedMessageQueue::OrderedMessageQueue() {
  orderedMessageQueue = list<ObjectMessageLetPair>();
}

OrderedMessageQueue::~OrderedMessageQueue() {
  // destroy all remaining inserted messages
  list<ObjectMessageLetPair>::iterator it = orderedMessageQueue.begin();
  list<ObjectMessageLetPair>::iterator end = orderedMessageQueue.end();
  while (it != end) {
    ObjectMessageLetPair omlPair = *it++;
    omlPair.second.first->freeMessage();
  }
}

void OrderedMessageQueue::insertMessage(MessageObject *messageObject, int outletIndex, PdMessage *message) {
  ObjectMessageLetPair omlPair = make_pair(messageObject, make_pair(message, outletIndex));
  
  list<ObjectMessageLetPair>::iterator it = orderedMessageQueue.begin();
  list<ObjectMessageLetPair>::iterator end = orderedMessageQueue.end();
  while (it != end) {
    if (message->getTimestamp() < it->second.first->getTimestamp()) {
      orderedMessageQueue.insert(it, omlPair);
      return;
    } else {
      ++it;
    }
  }
  orderedMessageQueue.push_back(omlPair); // insert at end
}

void OrderedMessageQueue::removeMessage(MessageObject *messageObject, int outletIndex, PdMessage *message) {
  list<ObjectMessageLetPair>::iterator it = orderedMessageQueue.begin();
  list<ObjectMessageLetPair>::iterator end = orderedMessageQueue.end();
  while (it != end) {
    ObjectMessageLetPair omlPair = *it;
    if (omlPair.first == messageObject &&
        omlPair.second.first == message &&
        omlPair.second.second == outletIndex) {
      orderedMessageQueue.erase(it);
      return;
    } else {
      ++it;
    }
  }
}

ObjectMessageLetPair OrderedMessageQueue::peek() {
  return orderedMessageQueue.front();
}

void OrderedMessageQueue::pop() {
  orderedMessageQueue.pop_front();
}

bool OrderedMessageQueue::empty() {
  return orderedMessageQueue.empty();
}
