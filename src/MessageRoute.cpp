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

#include "MessageRoute.h"

MessageRoute::MessageRoute(List *routeList, char *initString) : 
    MessageInputMessageOutputObject(1, routeList->getNumElements() + 1, initString) {
  this->routeList = routeList;
}

MessageRoute::~MessageRoute() {
  // do not delete messages that do not belong to this object
  messageOutletBuffers[routeList->getNumElements()]->clear();
  
  for (int i = 0; i < routeList->getNumElements(); i++) {
    delete (MessageElement *) routeList->get(i);
  }
  delete routeList;
}

void MessageRoute::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    for (int i = 0; i < routeList->getNumElements(); i++) {
      MessageElement *routeElement = (MessageElement *) routeList->get(i);
      if (messageElement->equals(routeElement)) {
        PdMessage *outgoingMessage = getNextOutgoingMessage(i);
        //outgoingMessage->clearAndCopyFrom(message);
        // NOTE: this is exactly like clearAndCopyInto() except that it 
        // starts with the 1st element and not the 0th.
        outgoingMessage->clear();
        for (int j = 1; j < message->getNumElements(); j++) {
          outgoingMessage->addElement(message->getElement(j)->copy());
        }
        outgoingMessage->setBlockIndex(message->getBlockIndex());
        return;
      }
    }
    // if no match could be found copy the incoming message to the output
    setNextOutgoingMessage(routeList->getNumElements(), message);
  }
}

PdMessage *MessageRoute::newCanonicalMessage() {
  return new PdMessage();
}
