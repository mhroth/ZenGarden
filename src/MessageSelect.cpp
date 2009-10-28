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

#include <stdlib.h>
#include "MessageElement.h"
#include "MessageSelect.h"

MessageSelect::MessageSelect(List *messageElementList, char *initString) : 
    MessageInputMessageOutputObject((messageElementList->getNumElements() == 0) ? 2 : 1, 
                                    (messageElementList->getNumElements() == 0) ? 2 : messageElementList->getNumElements()+1, initString) {
  this->messageElementList = messageElementList;
}

MessageSelect::~MessageSelect() {
  messageOutletBuffers[messageElementList->getNumElements()]->clear();
  
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    delete (MessageElement *) messageElementList->get(i);
  }
  delete messageElementList;
}

void MessageSelect::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      for (int i = 0; i < messageElementList->getNumElements(); i++) {
        MessageElement *selector = (MessageElement *) messageElementList->get(i);
        if (messageElement->equals(selector)) {
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
          return;
        }
      }

      setNextOutgoingMessage(messageElementList->getNumElements(), message);
      break;
    }
    case 1: {
      // TODO(mhroth): be able to set the selctor
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageSelect::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
