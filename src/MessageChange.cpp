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
#include "MessageChange.h"

MessageChange::MessageChange(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  left = 0.0f;
}

MessageChange::MessageChange(float initialValue, char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  left = initialValue;
}

MessageChange::~MessageChange() {
  // nothing to do
}

void MessageChange::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case FLOAT: {
          // output only if input is different than what is already there
          float input = messageElement->getFloat();
          if (input != left) {
            PdMessage *outgoingMessage = getNextOutgoingMessage(0);
            outgoingMessage->setBlockIndex(message->getBlockIndex());
            outgoingMessage->getElement(0)->setFloat(input);
            left = input;
          }
          break;
        }
        case BANG: {
          // force output
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
          outgoingMessage->getElement(0)->setFloat(left);
          break;
        }
        case SYMBOL: {
          if (strcmp(messageElement->getSymbol(), "set") == 0) {
            MessageElement *messageElement1 = message->getElement(1);
            if (messageElement1 != NULL && messageElement1->getType() == FLOAT) {
              left = messageElement1->getFloat();
            }
          }
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageChange::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  MessageElement *messageElement = new MessageElement(0.0f);
  message->addElement(messageElement);
  return message;
}
